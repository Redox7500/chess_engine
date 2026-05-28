#include <cstdint>
#include <array>
#include <vector>
#include <bitset>
#include <random>
#include <fstream>
#include <iostream>

constexpr uint8_t popcount_64(uint64_t x) {return __builtin_popcountll(x);}
constexpr uint8_t lsb_64(uint64_t x) {return __builtin_ctzll(x);}

constexpr uint8_t file_from_square(uint8_t square) {return square & 0b000111;}
constexpr uint8_t rank_from_square(uint8_t square) {return square >> 3;}
constexpr uint64_t bit_from_square(uint8_t square) {return uint64_t(1) << square;}
constexpr uint8_t to_square(uint8_t file, uint8_t rank) {return rank * 8 + file;}

void print_bitboard(uint64_t bitboard)
{
    for (uint8_t rank = 8; rank-- > 0; )
    {
        for (uint8_t file = 0; file < 8; file++)
        {
            std::cout << !!(bitboard & bit_from_square(to_square(file, rank))) << " ";
        }
        std::cout << "\n";
    }
}

uint64_t king_attack_mask(uint8_t square)
{
    uint64_t attack_mask = 0;

    const uint8_t file = file_from_square(square);
    const uint8_t rank = rank_from_square(square);

    attack_mask |= bit_from_square(to_square(file, rank - 1));
    attack_mask |= bit_from_square(to_square(file, rank + 1));
    if (file > 0)
    {
        attack_mask |= bit_from_square(to_square(file - 1, rank));
        if (rank > 0) {attack_mask |= bit_from_square(to_square(file - 1, rank - 1));}
        if (rank < 7) {attack_mask |= bit_from_square(to_square(file - 1, rank + 1));}
    }
    if (file < 7)
    {
        attack_mask |= bit_from_square(to_square(file + 1, rank));
        if (rank > 0) {attack_mask |= bit_from_square(to_square(file + 1, rank - 1));}
        if (rank < 7) {attack_mask |= bit_from_square(to_square(file + 1, rank + 1));}
    }

    return attack_mask;
}

uint64_t white_pawn_push_mask(uint8_t square)
{
    uint64_t attack_mask = 0;

    const uint8_t file = file_from_square(square);
    const uint8_t rank = rank_from_square(square);

    if (rank < 7)
    {
        attack_mask |= bit_from_square(to_square(file, rank + 1));
        if (rank == 1) {attack_mask |= bit_from_square(to_square(file, 3));}
    }

    return attack_mask;
}

uint64_t black_pawn_push_mask(uint8_t square)
{
    uint64_t attack_mask = 0;

    const uint8_t file = file_from_square(square);
    const uint8_t rank = rank_from_square(square);

    if (rank > 0)
    {
        attack_mask |= bit_from_square(to_square(file, rank - 1));
        if (rank == 6) {attack_mask |= bit_from_square(to_square(file, 4));}
    }

    return attack_mask;
}

uint64_t white_pawn_attack_mask(uint8_t square)
{
    uint64_t attack_mask = 0;

    const uint8_t file = file_from_square(square);
    const uint8_t rank = rank_from_square(square);

    if (rank < 7)
    {
        if (file > 0) {attack_mask |= bit_from_square(to_square(file - 1, rank + 1));}
        if (file < 7) {attack_mask |= bit_from_square(to_square(file + 1, rank + 1));}
    }

    return attack_mask;
}

uint64_t black_pawn_attack_mask(uint8_t square)
{
    uint64_t attack_mask = 0;

    const uint8_t file = file_from_square(square);
    const uint8_t rank = rank_from_square(square);

    if (rank > 0)
    {
        if (file > 0) {attack_mask |= bit_from_square(to_square(file - 1, rank - 1));}
        if (file < 7) {attack_mask |= bit_from_square(to_square(file + 1, rank - 1));}
    }

    return attack_mask;
}

uint64_t knight_attack_mask(uint8_t square)
{
    uint64_t attack_mask = 0;

    const uint8_t file = file_from_square(square);
    const uint8_t rank = rank_from_square(square);

    if (file > 0)
    {
        if (rank > 1) {attack_mask |= bit_from_square(to_square(file - 1, rank - 2));}
        if (rank < 6) {attack_mask |= bit_from_square(to_square(file - 1, rank + 2));}

        if (file > 1)
        {
            if (rank > 0) {attack_mask |= bit_from_square(to_square(file - 2, rank - 1));}
            if (rank < 7) {attack_mask |= bit_from_square(to_square(file - 2, rank + 1));}
        }
    }
    if (file < 7)
    {
        if (rank > 1) {attack_mask |= bit_from_square(to_square(file + 1, rank - 2));}
        if (rank < 6) {attack_mask |= bit_from_square(to_square(file + 1, rank + 2));}

        if (file < 6)
        {
            if (rank > 0) {attack_mask |= bit_from_square(to_square(file + 2, rank - 1));}
            if (rank < 7) {attack_mask |= bit_from_square(to_square(file + 2, rank + 1));}
        }
    }

    return attack_mask;
}

uint64_t bishop_attack_mask(uint8_t square, uint64_t blockers)
{
    uint64_t attack_mask = 0;

    const uint8_t file = file_from_square(square);
    const uint8_t rank = rank_from_square(square);

    for (uint8_t i = 1; file > i && rank > i; i++)
    {
        const uint8_t bit = bit_from_square(to_square(file - i, rank - i));
        attack_mask |= bit;

        if (blockers & bit) {break;}
    }
    for (uint8_t i = 1; file > i && rank + i < 7; i++)
    {
        const uint8_t bit = bit_from_square(to_square(file - i, rank + i));
        attack_mask |= bit;

        if (blockers & bit) {break;}
    }
    for (uint8_t i = 1; file + i < 7 && rank > i; i++)
    {
        const uint8_t bit = bit_from_square(to_square(file + i, rank - i));
        attack_mask |= bit;

        if (blockers & bit) {break;}
    }
    for (uint8_t i = 1; file + i < 7 && rank + i < 7; i++)
    {
        const uint8_t bit = bit_from_square(to_square(file + i, rank + i));
        attack_mask |= bit;

        if (blockers & bit) {break;}
    }

    return attack_mask;
}

uint64_t rook_attack_mask(uint8_t square, uint64_t blockers)
{
    uint64_t attack_mask = 0;

    const uint8_t file = file_from_square(square);
    const uint8_t rank = rank_from_square(square);

    for (uint8_t i = 1; i < file; i++)
    {
        const uint8_t bit = bit_from_square(to_square(i, rank));
        attack_mask |= bit;

        if (blockers & bit) {break;}
    }
    for (uint8_t i = file + 1; i < 7; i++)
    {
        const uint8_t bit = bit_from_square(to_square(i, rank));
        attack_mask |= bit;

        if (blockers & bit) {break;}
    }
    for (uint8_t i = 1; i < rank; i++)
    {
        const uint8_t bit = bit_from_square(to_square(file, i));
        attack_mask |= bit;

        if (blockers & bit) {break;}
    }
    for (uint8_t i = rank + 1; i < 7; i++)
    {
        const uint8_t bit = bit_from_square(to_square(file, i));
        attack_mask |= bit;

        if (blockers & bit) {break;}
    }

    return attack_mask;
}

uint64_t bishop_blocker_possibilities_mask(uint8_t square)
{
    uint64_t attack_mask = 0;

    const uint8_t file = file_from_square(square);
    const uint8_t rank = rank_from_square(square);

    for (uint8_t i = 1; file > i     && rank > i;     i++) {attack_mask |= bit_from_square(to_square(file - i, rank - i));}
    for (uint8_t i = 1; file > i     && rank + i < 7; i++) {attack_mask |= bit_from_square(to_square(file - i, rank + i));}
    for (uint8_t i = 1; file + i < 7 && rank > i;     i++) {attack_mask |= bit_from_square(to_square(file + i, rank - i));}
    for (uint8_t i = 1; file + i < 7 && rank + i < 7; i++) {attack_mask |= bit_from_square(to_square(file + i, rank + i));}

    return attack_mask;
}

uint64_t rook_blocker_possibilities_mask(uint8_t square)
{
    uint64_t attack_mask = 0;

    const uint8_t file = file_from_square(square);
    const uint8_t rank = rank_from_square(square);

    for (uint8_t i = 1;        i < file; i++) {attack_mask |= bit_from_square(to_square(i, rank));}
    for (uint8_t i = file + 1; i < 7;    i++) {attack_mask |= bit_from_square(to_square(i, rank));}
    for (uint8_t i = 1;        i < rank; i++) {attack_mask |= bit_from_square(to_square(file, i));}
    for (uint8_t i = rank + 1; i < 7;    i++) {attack_mask |= bit_from_square(to_square(file, i));}

    return attack_mask;
}

// uint64_t nth_subset(uint64_t mask, int index)
// {
//     uint64_t subset = 0;
//     int bit_index = 0;

//     while (mask)
//     {
//         int square = __builtin_ctzll(mask);
//         mask &= mask - 1;

//         if (index & bit_from_square(bit_index))
//         {
//             subset |= bit_from_square(square);
//         }

//         bit_index++;
//     }

//     return subset;
// }

std::vector<uint64_t> all_blocker_bitboards(uint64_t naive_attack_mask)
{
    const uint8_t bits = popcount_64(naive_attack_mask);
    const uint16_t size = 1 << bits;

    std::vector<uint64_t> blocker_bitboards(size);
    uint64_t blocker_bitboard = naive_attack_mask;
    for (uint16_t i = 0; i < size && blocker_bitboard > 0; i++, blocker_bitboard = (blocker_bitboard - 1) & naive_attack_mask)
    {
        blocker_bitboards.push_back(blocker_bitboard);
    }

    return blocker_bitboards;
}

uint64_t random_magic(std::mt19937_64& rng)
{
    return rng() & rng();
}

struct magic_data
{
    uint64_t magic;
    std::vector<uint64_t> attack_masks;
};

magic_data find_magic(const std::vector<uint64_t>& blocker_bitboards, const std::vector<uint64_t>& attack_bitboards, uint8_t bits)
{
    const uint16_t size = 1 << bits;

    std::vector<uint64_t> used(size);

    std::mt19937_64 rng(0x511);
    while (true)
    {
        uint64_t magic = random_magic(rng);

        // if (__builtin_popcountll((magic * mask) >> (64 - bits)) < 6)
        // {
        //     continue;
        // }

        std::fill(used.begin(), used.end(), 0);

        bool fail = false;
        for (uint16_t i = 0; i < size; i++)
        {
            print_bitboard(blocker_bitboards[i]);
            std::cout << "\n";
            uint16_t index = (blocker_bitboards[i] * magic) >> (64 - bits);
            if (index >= size || used[index] != attack_bitboards[i])
            {
                fail = true;
                break;
            }
            if (!used[index])
            {
                used[index] = attack_bitboards[i];
            }
        }

        if (!fail)
        {
            return {magic, used};
        }
    }
}

template <typename T, size_t L>
void dump_array(std::ofstream& out, const char* name, const char* datatype, const char* suffix, const std::array<T, L>& array)
{
    size_t size = array.size();
    out << "constexpr " << datatype << " " << name << "[" << size << "] = {";
    if (size)
    {
        out << "\n";
        for (size_t i = 0; i < size; i++)
        {
            out << "    " << array[i] << suffix;

            if (i != size - 1)
            {
                out << ",";
            }
            out << "\n";
        }
    }
    out << "};\n\n";
};

template <typename T>
void dump_nested_vector(std::ofstream& out, const char* name, const char* datatype, const char* suffix, const std::vector<std::vector<T>>& vector)
{
    out << "constexpr std::vector<std::vector<" << datatype << ">> " << name << " = {";

    const size_t vector_size = vector.size();
    if (vector_size)
    {
        out << "\n";
        for (size_t i = 0; i < vector_size; i++)
        {
            const size_t inner_vector_size = vector[i].size();

            out << "    {\n";
            for (size_t j = 0; j < inner_vector_size - 1; j++)
            {
                out << vector[i][j] << ",\n";
            }
            out << vector[i][inner_vector_size - 1] << "\n}";

            if (i != vector_size - 1)
            {
                out << ",";
            }
            out << "\n";
        }
    }
    out << "};\n\n";
}

int main()
{
    std::array<uint64_t, 64> knight_attack_masks, king_attack_masks, bishop_magics, rook_magics;
    std::vector<std::vector<uint64_t>> ordered_bishop_attack_masks(64), ordered_rook_attack_masks(64);
    std::array<uint8_t, 64> rook_shifts, bishop_shifts;

    for (uint8_t square = 0; square < 64; square++)
    {
        knight_attack_masks[square] = knight_attack_mask(square);
        king_attack_masks[square] = king_attack_mask(square);

        const uint64_t current_bishop_blocker_possibilities_mask = bishop_blocker_possibilities_mask(square);
        const uint64_t current_rook_blocker_possibilities_mask = rook_blocker_possibilities_mask(square);
        std::vector<uint64_t> corresponding_bishop_attack_masks;
        std::vector<uint64_t> corresponding_rook_attack_masks;
        
        std::vector<uint64_t> bishop_blocker_bitboards = all_blocker_bitboards(current_bishop_blocker_possibilities_mask);
        std::vector<uint64_t> rook_blocker_bitboards = all_blocker_bitboards(current_rook_blocker_possibilities_mask);

        for (uint64_t blocker_bitboard:bishop_blocker_bitboards)
        {
            corresponding_bishop_attack_masks.push_back(bishop_attack_mask(square, blocker_bitboard));
        }
        for (uint64_t blocker_bitboard:rook_blocker_bitboards)
        {
            corresponding_rook_attack_masks.push_back(rook_attack_mask(square, blocker_bitboard));
        }

        uint8_t bishop_bits = popcount_64(current_bishop_blocker_possibilities_mask);
        uint8_t rook_bits = popcount_64(current_rook_blocker_possibilities_mask);

        bishop_shifts[square] = 64 - bishop_bits;
        rook_shifts[square] = 64 - rook_bits;

        bishop_magics[square] = find_magic(bishop_blocker_bitboards, corresponding_bishop_attack_masks, bishop_bits).magic;
        rook_magics[square] = find_magic(rook_blocker_bitboards, corresponding_rook_attack_masks, rook_bits).magic;

        std::cout << "Square " << square << " done\n";
    }

    std::ofstream out("precomputed_bitboards.h");
    out << "#pragma once\n#include <cstdint>\n\n";

    dump_array(out, "knight_attack_masks"      , "uint64_t"    , "ULL", knight_attack_masks      );
    dump_array(out, "king_attack_masks"        , "uint64_t"    , "ULL", king_attack_masks        );
    dump_array(out, "bishop_magics"            , "uint64_t"    , "ULL", bishop_magics            );
    dump_array(out, "rook_magics"              , "uint64_t"    , "ULL", rook_magics              );
    dump_array(out, "bishop_shifts"            , "unsigned int", "U"  , bishop_shifts            );
    dump_array(out, "rook_shifts"              , "unsigned int", "U"  , rook_shifts              );
    dump_nested_vector(out, "bishop_attack_masks", "uint64_t", "ULL", ordered_bishop_attack_masks);
    dump_nested_vector(out, "rook_attack_masks"  , "uint64_t", "ULL", ordered_rook_attack_masks  );

    std::cout << "Done. precomputed_bitboards.h written";
}