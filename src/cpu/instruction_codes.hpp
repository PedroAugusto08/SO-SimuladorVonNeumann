// instruction_codes.hpp
// Fonte única de verdade para opcodes/funct e formato das instruções
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <stdexcept>

namespace instr {

enum class Format { R, I, J, SPECIAL };

struct InstrCode {
    uint8_t opcode;   // 6 bits
    uint8_t funct;    // 6 bits (apenas R-type)
    Format format;
};

inline const std::unordered_map<std::string, InstrCode>& table() {
    static const std::unordered_map<std::string, InstrCode> T = {
        // R-type (MIPS-like)
        {"add",  {0x00, 0x20, Format::R}},
        {"sub",  {0x00, 0x22, Format::R}},
        {"and",  {0x00, 0x24, Format::R}},
        {"or",   {0x00, 0x25, Format::R}},
        {"mult", {0x00, 0x18, Format::R}},
        {"div",  {0x00, 0x1A, Format::R}},
        {"sll",  {0x00, 0x00, Format::R}},
        {"srl",  {0x00, 0x02, Format::R}},
        {"jr",   {0x00, 0x08, Format::R}},

        // I-type (MIPS-like + custom blt/bgt)
        {"addi", {0x08, 0x00, Format::I}},
        {"andi", {0x0C, 0x00, Format::I}},
        {"ori",  {0x0D, 0x00, Format::I}},
        {"slti", {0x0A, 0x00, Format::I}},
        {"lw",   {0x23, 0x00, Format::I}},
        {"sw",   {0x2B, 0x00, Format::I}},
        {"beq",  {0x04, 0x00, Format::I}},
        {"bne",  {0x05, 0x00, Format::I}},
        // Custom (compatível com parser atual)
        {"bgt",  {0x07, 0x00, Format::I}},
        {"blt",  {0x09, 0x00, Format::I}},

        // J-type
        {"j",    {0x02, 0x00, Format::J}},
        {"jal",  {0x03, 0x00, Format::J}},

        // Especiais
        // print/end definidos no parser como 0x3E e 0x3F
        {"print",{0x3E, 0x00, Format::SPECIAL}},
        {"end",  {0x3F, 0x00, Format::SPECIAL}},

        // Aliases úteis
        {"li",   {0x08, 0x00, Format::I}}, // li -> addi $rt, $zero, imm
    };
    return T;
}

inline bool exists(const std::string& name) {
    return table().find(name) != table().end();
}

inline InstrCode get(const std::string& name) {
    auto s = name;
    // normaliza para lower
    for (auto &c : s) c = static_cast<char>(::tolower(c));
    auto it = table().find(s);
    if (it == table().end()) throw std::runtime_error("Instrucao desconhecida: " + name);
    return it->second;
}

inline std::string nameFromOpcodeFunct(uint8_t opcode, uint8_t funct) {
    // Busca reversa simples (usada apenas em depuração/decodificação)
    for (const auto &p : table()) {
        if (p.second.opcode == opcode) {
            if (p.second.format == Format::R) {
                if (p.second.funct == funct) return p.first;
            } else {
                return p.first;
            }
        }
    }
    return std::string();
}

} // namespace instr
