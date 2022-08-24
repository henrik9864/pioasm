/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <algorithm>
#include <iostream>
#include "output_format.h"
#include "pio_disassembler.h"

struct c_output : public output_format {
    struct factory {
        factory() {
            output_format::add(new c_output());
        }
    };

    c_output() : output_format("c") {}

    std::string get_description() override {
        return "C header instructions for PIO";
    }

    void output_symbols(FILE *out, std::string prefix, const std::vector<compiled_source::symbol> &symbols) {
        int count = 0;
        for (const auto &s : symbols) {
            if (!s.is_label) {
                fprintf(out, "#define %s%s %d\n", prefix.c_str(), s.name.c_str(), s.value);
                count++;
            }
        }
        if (count) {
            fprintf(out, "\n");
            count = 0;
        }
        for (const auto &s : symbols) {
            if (s.is_label) {
                fprintf(out, "#define %soffset_%s %du\n", prefix.c_str(), s.name.c_str(), s.value);
                count++;
            }
        }
        if (count) {
            fprintf(out, "\n");
        }
    }

    void header(FILE *out, std::string msg) {
        std::string dashes = std::string(msg.length(), '-');
        fprintf(out, "// %s //\n", dashes.c_str());
        fprintf(out, "// %s //\n", msg.c_str());
        fprintf(out, "// %s //\n", dashes.c_str());
        fprintf(out, "\n");
    }

    int output(std::string destination, std::vector<std::string> output_options,
               const compiled_source &source) override {

        for (const auto &program : source.programs) {
            for(const auto &p : program.lang_opts) {
                if (p.first.size() >= name.size() && p.first.compare(0, name.size(), name) == 0) {
                    std::cerr << "warning: " << name << " does not support output options; " << p.first << " lang_opt ignored.\n";
                }
            }
        }
        FILE *out = open_single_output(destination);
        if (!out) return 1;

        header(out, "This file is autogenerated by pioasm; do not edit!");

        output_symbols(out, "", source.global_symbols);

        for (const auto &program : source.programs) {
            header(out, program.name);

            std::string prefix = program.name + "_";

            fprintf(out, "#define %swrap_target %d\n", prefix.c_str(), program.wrap_target);
            fprintf(out, "#define %swrap %d\n", prefix.c_str(), program.wrap);
            fprintf(out, "\n");

            output_symbols(out, prefix, program.symbols);

            if (output_options.size() > 0)
            {
                fprintf(out, "__attribute__((section(\".%s\")))", output_options.at(0).c_str());
            }

            fprintf(out, "static const uint16_t %sprogram_instructions[] = {\n", prefix.c_str());
            for (int i = 0; i < (int)program.instructions.size(); i++) {
                const auto &inst = program.instructions[i];
                if (i == program.wrap_target) {
                    fprintf(out, "            //     .wrap_target\n");
                }
                fprintf(out, "    0x%04x, // %2d: %s\n", inst, i,
                        disassemble(inst, program.sideset_bits_including_opt.get(), program.sideset_opt).c_str());
                if (i == program.wrap) {
                    fprintf(out, "            //     .wrap\n");
                }
            }
            fprintf(out, "};\n");
            fprintf(out, "\n");
        }
        if (out != stdout) { fclose(out); }
        return 0;
    }
};

static c_output::factory creator;
