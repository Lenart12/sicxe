#include <fstream>
#include <iostream>
#include <iomanip>
#include <functional>
#include <ranges>
#include "sim/ObjLoader.h"
#include "sim/Machine.h"
#include "sim/Disassembler.h"
#include "asm/Parser.h"
#include "asm/SicCST.h"

class MachineController {
public:
    explicit MachineController(std::unique_ptr<Machine> machine)
    : m_disassembler(Disassembler {machine->get_memory()})
    , m_registers(machine->get_registers())
    , m_changes(machine->get_changes())
    , m_memory(machine->get_memory())
    , m_machine(std::move(machine))
    {}

private:
    struct Command {
        std::string_view name;
        std::string_view help_text;
        std::function<void(std::optional<int>)> callback;
    };

    void initialize_commands() {
        m_commands.push_back({"run", ": Run a program until a breakpoint or is halted", [&] (auto) {
            m_machine->run();

            if (m_machine->in_halt_condition()) {
                std::cout << "Program halted" << std::endl;
            } else {
                std::cout << "Breakpoint at [";
                print_zero_hex(6, m_registers.getPc());
                std::cout << "]" << std::endl;
            }
            print_pc_disassembly();
        }});
        m_commands.push_back({"step", " [n = 1]: Run a program for n steps", [&] (auto maybe_step_count) {
            int step_count = maybe_step_count.value_or(1);
            for (size_t i = 0; i < step_count; i++) {
                print_pc_disassembly();
                m_machine->step();
                if (m_machine->in_halt_condition()) {
                    break;
                }
            }
        }});
        m_commands.push_back({"undo", " [n = 1]: Undo last n steps", [&] (auto maybe_step_count) {
            int step_count = maybe_step_count.value_or(1);
            for (size_t i = 0; i < step_count && m_machine->can_undo(); i++) {
                m_machine->undo();
                print_pc_disassembly();
            }
        }});
        m_commands.push_back({"history", " [max steps back = all]: show step history", [&] (auto maybe_max_steps) {
            int steps_back = 0;
            int max_steps = maybe_max_steps.value_or(m_changes.size());
            bool reached_end = true;

            for (auto change : std::ranges::reverse_view(m_changes)) {
                std::visit([&](auto&& arg) {
                    if (-steps_back >= max_steps) {
                        reached_end = false;
                        return;
                    };

                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, RegisterChange> || std::is_same_v<T, MemoryChange>) {
                        std::cout << arg << std::endl;
                    }
                    else if constexpr (std::is_same_v<T, Machine::ChangeStart>) {
                        std::cout << std::dec << std::setw(2) << --steps_back << ": "; print_disassembly(arg.pc);
                        std::cout << std::endl;
                    }
                }, change);
            }

            if (reached_end) std::cout << "End of history" << std::endl;
        }}) ;
        m_commands.push_back({"registers", ": Show current register state", [&] (auto) {

            auto show_register = [&](Register r) {
                auto reg = m_registers.get(r);
                print_zero_hex(6, reg & 0xffffff);
                std::cout << " (" << std::dec << reg << ")" << std::endl;
            };

            std::cout << "A:  "; show_register(Register::A);
            std::cout << "X:  "; show_register(Register::X);
            std::cout << "L:  "; show_register(Register::L);
            std::cout << "B:  "; show_register(Register::B);
            std::cout << "S:  "; show_register(Register::S);
            std::cout << "T:  "; show_register(Register::T);
            std::cout << "F:  "; show_register(Register::F);
            std::cout << "PC: "; show_register(Register::PC);
            std::cout << "SW: "; show_register(Register::SW);
            std::cout << "CC: "; show_register(Register::CC);
            if (m_registers.CC_is_lower()) std::cout << "Less than" << std::endl;
            else if (m_registers.CC_is_greater()) std::cout << "Greater than" << std::endl;
            else if (m_registers.CC_is_equal()) std::cout << "Equal" << std::endl;
            else  std::cout << std::setw(6) << std::hex << m_registers.getCC() << std::endl;
        }});
        m_commands.push_back({"byte", " (address): Show byte at address", [&] (auto maybe_address) {
            int address = maybe_address.value_or(-1);

            if (address < 0 || address > Memory::mem_size) {
                std::cout << "Invalid memory address [" << address << "]" << std::endl;
                return;
            }

            std::cout << "["; print_zero_hex(6, address); std::cout << "] = ";
            std::cout << "["; print_zero_hex(2, m_memory->get_byte(address));
            std::cout << "]" << std::endl;
        }});
        m_commands.push_back({"word", " (address): Show word at address", [&] (auto maybe_address) {
            int address = maybe_address.value_or(-1);

            if (address < 0 || address >= Memory::mem_size) {
                std::cout << "Invalid memory address [" << address << "]" << std::endl;
                return;
            }

            std::cout << "["; print_zero_hex(6, address); std::cout << "] = ";
            std::cout << "["; print_zero_hex(6, m_memory->get_word(address));
            std::cout << "]" << std::endl;
        }});
        m_commands.push_back({"disassemble", " [address = PC]: Disassemble instruction at address", [&] (auto maybe_address) {
            int address = maybe_address.value_or(m_registers.getPc());

            if (address < 0 || address >= Memory::mem_size) {
                std::cout << "Invalid memory address [" << address << "]" << std::endl;
                return;
            }

            print_disassembly(address);
        }});
        m_commands.push_back({"breakpoint", " (address): Set breakpoint at address", [&] (auto maybe_address) {
            int address = maybe_address.value_or(-1);

            if (address < 0 || address >= Memory::mem_size) {
                std::cout << "Invalid memory address [" << address << "]" << std::endl;
                return;
            }

            m_machine->set_execution_breakpoint(address);
            std::cout << "Set breakpoint at "; print_zero_hex(6, address); std::cout << std::endl;
        }});
        m_commands.push_back({"nobreakpoint", " (address): Unset at address", [&] (auto maybe_address) {
            int address = maybe_address.value_or(-1);

            if (address < 0 || address >= Memory::mem_size) {
                std::cout << "Invalid memory address [" << address << "]" << std::endl;
                return;
            }

            m_machine->clear_execution_breakpoint(address);
            std::cout << "Cleared breakpoint at 0x"; print_zero_hex(6, address); std::cout << std::endl;
        }});
        m_commands.push_back({"clearbreakpoint", ": Clear all breakpoints", [&] (auto maybe_address) {
            int address = maybe_address.value_or(-1);

            if (address < 0 || address >= Memory::mem_size) {
                std::cout << "Invalid memory address [" << address << "]" << std::endl;
                return;
            }

            m_machine->clear_execution_breakpoints();
            std::cout << "Cleared breakpoints" << std::endl;
        }});
        m_commands.push_back({"exit", ": Exit the program", [&] (auto) {
            exit(0);
        }});
        m_commands.push_back({"help", ": Show help (this)", [&](auto) {
            print_help();
        }});
    }

public:
    [[noreturn]] void run() {
        initialize_commands();
        std::cout << "Program loaded" << std::endl;
        std::cout << "Use 'help' to find out commands" << std::endl;

        while (true) {
            std::string line;
            std::getline(std::cin, line);
            auto maybe_command = get_command(line);

            if (maybe_command.has_value()) {
                auto maybe_number = get_number(line);
                if (maybe_number.has_value() && address_is_pc_relative(line)) {
                    *maybe_number += m_registers.getPc();
                }
                maybe_command->callback(maybe_number);
            } else {
                std::cout << "Invalid command [" << line << "] - use 'help'" << std::endl;
            }
        }
    }
private:
    void print_help() {
        std::cout << "Sic/Xe simulator v1.0" << std::endl;
        std::cout << "Each command can be called by its shortest non-ambiguous name" << std::endl;
        std::cout << "Format: command [optional = default] (required)" << std::endl;
        std::cout << "        '*' at the start off the command will offset address parameter with PC" << std::endl;
        std::cout << "Commands: " << std::endl;

        auto show_command_help = [] (const Command& command) {
            std::cout << "  - " << command.name << command.help_text << std::endl;
        };

        std::for_each(m_commands.begin(), m_commands.end(), show_command_help);
    }
    static void print_hex(int width, int word) {
        std::cout << std::setfill('0') << std::setw(width) << std::hex << word;
    };
    static void print_zero_hex(int width, int word) {
        std::cout << "0x";
        print_hex(width, word);
    };
    void print_disassembly(Address_t target_address) {
        auto disassembly = m_disassembler.dissasemble_at(target_address);

        std::cout << "["; print_zero_hex(6, target_address); std::cout << "]: ";
        std::cout << disassembly << std::endl;
    };
    void print_pc_disassembly() {
        print_disassembly(m_registers.getPc());
    }
    std::optional<Command> get_command(const std::string& line) {
        if (line.empty()) return std::nullopt;

        auto pc_relative_start_offset = address_is_pc_relative(line) * 1;
        auto command_str = line.substr(pc_relative_start_offset, line.find(' ') - pc_relative_start_offset);
        std::optional<Command> selected {};
        for (Command& command : m_commands) {
            if (command.name.starts_with(command_str)) {
                // Ambiguous command
                if (selected.has_value())
                    return std::nullopt;

                selected = command;
            }
        }

        return selected;
    }
    static std::optional<int> get_number(const std::string& line) {
        if (line.empty()) return std::nullopt;

        auto space_index = line.find(' ');

        if (space_index == std::string::npos) return std::nullopt;

        auto number_str = line.substr(space_index + 1);

        auto is_hex = number_str.starts_with("0x");
        auto is_negative_hex = number_str.starts_with("-0x");
        size_t start_pos = 0;
        if (is_hex) start_pos = 2;
        else if (is_negative_hex) start_pos = 3;
        try {
            auto num = std::stoi(number_str, &start_pos, is_hex || is_negative_hex ? 16u : 10u);
            return is_negative_hex ? -num : num;
        } catch (std::invalid_argument&) {
            return std::nullopt;
        }
    }
    static bool address_is_pc_relative(const std::string& line) {
        return line.at(0) == '*';
    }
private:
    Disassembler m_disassembler;

    const Registers& m_registers;
    const std::deque<Machine::Change_t>& m_changes;
    std::shared_ptr<Memory> m_memory;
    std::unique_ptr<Machine> m_machine;
    std::vector<Command> m_commands {};
};

int sim_main(std::vector<std::string> args) {
    std::string file_name {};
    if (args.size() < 2) {
        file_name = "../test_programs/addr.obj";
    } else {
        file_name = args[0];
    }

    auto stream = std::ifstream {file_name};

    if (!stream.is_open()) {
        std::cout << "Cant open " << file_name << std::endl;
        return 1;
    }

    auto memory = std::make_shared<Memory>();
    auto loader = ObjLoader {memory, stream};
    auto machine = std::make_unique<Machine>(loader.load_obj(), memory);

    MachineController{std::move(machine)}.run();
}

int asm_main(std::vector<std::string> args) {
    std::string file_name {};
    std::string output_filename {};
    std::string list_filename {};
    if (args.size() < 2) {
        file_name = "../test_programs/tix.asm";
    } else {
        if (args.size() > 4) {
            std::cout << "(input_filename) [output_filename = stdout] [list_filename = stdout]";
            exit(1);
        }
        file_name = args[1];
        if (args.size() >= 3)
            output_filename = args[2];
        if (args.size() >= 4)
            list_filename = args[3];
    }

    auto stream = std::ifstream {file_name};

    if (!stream.is_open()) {
        std::cout << "Cant open " << file_name << std::endl;
        return 1;
    }

    optional<std::ofstream> obj_stream;
    optional<std::ofstream> lst_stream;

    if (!output_filename.empty()) {
        obj_stream = std::ofstream {output_filename};

        if (!obj_stream.value().is_open()) {
            std::cout << "Cant open " << output_filename << std::endl;
            return 1;
        }
    }

    if (!list_filename.empty()) {
        lst_stream = std::ofstream {list_filename};

        if (!lst_stream.value().is_open()) {
            std::cout << "Cant open " << list_filename << std::endl;
            return 1;
        }
    }

    auto file_content = std::string {std::istreambuf_iterator<char>(stream), {}};

    auto lexer = Lexer { file_content };

    std::vector<Token> tokens;

    while (true) {
        auto maybe_token = lexer.next();
        if (!maybe_token.has_value()) break;

        tokens.push_back(std::move(maybe_token.value()));
    }

    auto parser = Parser { tokens };

    auto program = parser.parse();

    auto assembler = ProgramAssembler { program };

    if (obj_stream.has_value()) {
        assembler.set_obj_stream(&obj_stream.value());
    }

    if (lst_stream.has_value()) {
        assembler.set_lst_stream(&lst_stream.value());
    }

    assembler.assemble_program();

    return 0;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args;

    for (int i = 0; i < argc; i++) args.emplace_back(argv[i]);

    return asm_main(std::move(args));
//    return sim_main(std::move(args));
}
