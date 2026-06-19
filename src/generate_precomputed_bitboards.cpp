#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <vector>

#include <cstddef>
#include <cstdint>

using Bitboard  = std::uint64_t;
using Square    = std::uint8_t;
using Coord     = std::uint8_t;
using Offset    = std::int8_t;
using Position  = std::array<Coord,  2>;
using Direction = std::array<Offset, 2>;

struct MagicData
{
    Bitboard magic;
    std::vector<Bitboard> attacks_bitboards;
};

constexpr std::array<Direction, 4> bishop_directions{{
    {{-1, -1}},
    {{-1,  1}},
    {{ 1, -1}},
    {{ 1,  1}}
}};
constexpr std::array<Direction, 4> rook_directions{{
    {{-1,  0}},
    {{ 0, -1}},
    {{ 0,  1}},
    {{ 1,  0}}
}};

constexpr unsigned int popcount_64(Bitboard x) {return __builtin_popcountll(x);}
template <typename T> constexpr unsigned int ctz_log2(T x) {return __builtin_ctz(x);}

constexpr Bitboard bitboard_from_square(Square square) {return 1 << square;}
constexpr Coord file_from_square(Square square) {return square & 0b000111;}
constexpr Coord rank_from_square(Square square) {return square >> 3;}
constexpr Position position_from_square(Square square) {return {file_from_square(square), rank_from_square(square)};}

constexpr Square square_from_coords(Coord file, Coord rank) {return rank * 8 + file;}
constexpr Square square_from_position(const Position& position) {return square_from_coords(position[0], position[1]);}

constexpr Position add_direction(const Position& position, const Direction& direction) {return {static_cast<Coord>(position[0] + direction[0]), static_cast<Coord>(position[1] + direction[1])};}

template <typename T>
constexpr void append_vector(std::vector<T>& to_be_appended_to, std::vector<T>& to_append) 
{
    to_be_appended_to.reserve(to_be_appended_to.size() + to_append.size());
    std::move(to_append.begin(), to_append.end(), std::back_inserter(to_be_appended_to));
    to_append.clear();
}

void print_bitboard(Bitboard bitboard)
{
    for (Coord rank = 8; rank-- > 0; )
    {
        for (Coord file = 0; file < 8; file++)
        {
            std::cout << !!(bitboard & bitboard_from_square(square_from_coords(file, rank))) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

Bitboard king_attacks_bitboard(Square square)
{
    Bitboard attacks_bitboard = 0;

    const Coord file = file_from_square(square);
    const Coord rank = rank_from_square(square);

    attacks_bitboard |= bitboard_from_square(square_from_coords(file, rank - 1));
    attacks_bitboard |= bitboard_from_square(square_from_coords(file, rank + 1));
    if (file > 0)
    {
        attacks_bitboard |= bitboard_from_square(square_from_coords(file - 1, rank));
        if (rank > 0) {attacks_bitboard |= bitboard_from_square(square_from_coords(file - 1, rank - 1));}
        if (rank < 7) {attacks_bitboard |= bitboard_from_square(square_from_coords(file - 1, rank + 1));}
    }
    if (file < 7)
    {
        attacks_bitboard |= bitboard_from_square(square_from_coords(file + 1, rank));
        if (rank > 0) {attacks_bitboard |= bitboard_from_square(square_from_coords(file + 1, rank - 1));}
        if (rank < 7) {attacks_bitboard |= bitboard_from_square(square_from_coords(file + 1, rank + 1));}
    }

    return attacks_bitboard;
}

Bitboard knight_attacks_bitboard(Square square)
{
    Bitboard attacks_bitboard = 0;

    const Coord file = file_from_square(square);
    const Coord rank = rank_from_square(square);

    if (file > 0)
    {
        if (rank > 1) {attacks_bitboard |= bitboard_from_square(square_from_coords(file - 1, rank - 2));}
        if (rank < 6) {attacks_bitboard |= bitboard_from_square(square_from_coords(file - 1, rank + 2));}

        if (file > 1)
        {
            if (rank > 0) {attacks_bitboard |= bitboard_from_square(square_from_coords(file - 2, rank - 1));}
            if (rank < 7) {attacks_bitboard |= bitboard_from_square(square_from_coords(file - 2, rank + 1));}
        }
    }
    if (file < 7)
    {
        if (rank > 1) {attacks_bitboard |= bitboard_from_square(square_from_coords(file + 1, rank - 2));}
        if (rank < 6) {attacks_bitboard |= bitboard_from_square(square_from_coords(file + 1, rank + 2));}

        if (file < 6)
        {
            if (rank > 0) {attacks_bitboard |= bitboard_from_square(square_from_coords(file + 2, rank - 1));}
            if (rank < 7) {attacks_bitboard |= bitboard_from_square(square_from_coords(file + 2, rank + 1));}
        }
    }

    return attacks_bitboard;
}

Bitboard slider_blocker_possibilities_bitboard(Square square, const std::array<Direction, 4> directions)
{
    Bitboard attacks_bitboard = 0;
    const Position start_position = position_from_square(square);

    for (const Direction direction:directions)
    {
        for (
            Position current_position = add_direction(start_position, direction);
            0 < current_position[0] && current_position[0] < 7 && 0 < current_position[1] && current_position[1] < 7;
            current_position = add_direction(current_position, direction)
        ) {attacks_bitboard |= bitboard_from_square(square_from_position(current_position));}
    }

    return attacks_bitboard;
}

Bitboard slider_attacks_bitboard(Square square, Bitboard blockers_bitboard, const std::array<Direction, 4> directions)
{
    Bitboard attacks_bitboard = 0;
    const Position start_position = position_from_square(square);
    
    for (const Direction direction:directions)
    {
        Position current_position = add_direction(start_position, direction);
        Bitboard current_bit = 0;
        while (!(blockers_bitboard & current_bit) && current_position[0] < 8 && current_position[1] < 8)
        {
            current_bit = bitboard_from_square(square_from_position(current_position));
            attacks_bitboard |= current_bit;

            current_position = add_direction(current_position, direction);
        }
    }

    return attacks_bitboard;
}

std::vector<Bitboard> all_blockers_bitboards(Bitboard blocker_possibilities_bitboard)
{
    const unsigned int bits = popcount_64(blocker_possibilities_bitboard);
    const std::size_t size = 1 << bits;

    std::vector<Bitboard> blockers_bitboards;
    blockers_bitboards.reserve(size);

    Bitboard blockers_bitboard = 0;
    for (std::size_t i = 0; i < size; i++)
    {
        blockers_bitboards.push_back(blockers_bitboard);
        blockers_bitboard = (blockers_bitboard - 1) & blocker_possibilities_bitboard;
    }

    return blockers_bitboards;
}

Bitboard random_magic(std::mt19937_64& rng)
{
    return rng() & rng() & rng();
}

MagicData find_magic(const std::vector<Bitboard>& blockers_bitboards, const std::vector<Bitboard>& attacks_bitboards)
{
    const std::size_t size = blockers_bitboards.size();
    if (!size) {std::cerr << "empty blockers bitboards vector" << std::endl; return {};}
    if (size != attacks_bitboards.size()) {std::cerr << "mismatched blockers bitboards and attacks bitboards; each blockers bitboard should match with each attacks bitboard 1:1" << std::endl; return {};}

    if (size & (size - 1)) {std::cerr << "blockers bitboards vector size is not an integer power of 2" << std::endl; return{};}
    
    const unsigned int bits = ctz_log2(size);
    if (bits == 0 || bits > 63) {std::cerr << "blockers bitboards vector has size = 1 or size > 2^63" << std::endl; return {};}


    std::vector<Bitboard> ordered_attacks_bitboards(size);
    std::vector<bool> set_ordered_attacks_bitboards(size);
    std::mt19937_64 rng(511);

    while (true)
    {
        Bitboard magic = random_magic(rng);

        bool fail = false;

        for (std::size_t i = 0; i < size; i++)
        {
            // print_bitboard(blockers_bitboards[i]);
            const std::size_t index = (blockers_bitboards[i] * magic) >> (64 - bits);
            if (set_ordered_attacks_bitboards[index])
            {
                if (ordered_attacks_bitboards[index] != attacks_bitboards[i])
                {
                    fail = true;
                    std::fill(ordered_attacks_bitboards    .begin(), ordered_attacks_bitboards    .end(), 0);
                    std::fill(set_ordered_attacks_bitboards.begin(), set_ordered_attacks_bitboards.end(), false);
                    std::cout << "failed after " << i << " blocker bitboards\n";
                    break;
                }
            }
            else
            {
                ordered_attacks_bitboards    [index] = attacks_bitboards[i];
                set_ordered_attacks_bitboards[index] = true;
            }
        }

        if (!fail)
        {
            std::cout << "correct magic found!\n";
            return {magic, ordered_attacks_bitboards};
        }
    }
}

template <typename T, std::size_t L>
void dump_array(std::ofstream& out, const char* name, const char* datatype, const char* suffix, const std::array<T, L>& array)
{
    const std::size_t size = array.size();

    out << "constexpr std::array<" << datatype << ", " << size << "> " << name << " = {";
    if (size)
    {
        out << "\n";
        for (std::size_t i = 0; i < size; i++)
        {
            out << "\t" << array[i] << suffix;

            if (i != size - 1)
            {
                out << ",";
            }
            out << "\n";
        }
    }
    out << "};\n";
};

template <typename T>
void dump_array(std::ofstream& out, const char* name, const char* datatype, const char* suffix, const std::vector<T>& array)
{
    const std::size_t size = array.size();

    out << "constexpr std::array<" << datatype << ", " << size << "> " << name << " = {";
    if (size)
    {
        out << "\n";
        for (std::size_t i = 0; i < size; i++)
        {
            out << "\t" << array[i] << suffix;

            if (i != size - 1)
            {
                out << ",";
            }
            out << "\n";
        }
    }
    out << "};\n";
}

int main()
{
    std::array<Bitboard, 64> knight_attacks_bitboards, king_attacks_bitboards;
    std::array<Bitboard, 64> bishop_magic_bitboards, rook_magic_bitboards;
    std::array<unsigned int, 64> rook_shifts, bishop_shifts; 
    std::vector<Bitboard> ordered_bishop_attacks_bitboards, ordered_rook_attacks_bitboards;
    std::array<std::size_t, 64> ordered_bishop_attacks_bitboards_row_offsets, ordered_rook_attacks_bitboards_row_offsets;

    for (Square square = 0; square < 64; square++)
    {
        knight_attacks_bitboards[square] = knight_attacks_bitboard(square);
        king_attacks_bitboards  [square] = king_attacks_bitboard  (square);

        const Bitboard current_bishop_blocker_possibilities_bitboard = slider_blocker_possibilities_bitboard(square, bishop_directions);
        const Bitboard current_rook_blocker_possibilities_bitboard   = slider_blocker_possibilities_bitboard(square, rook_directions);
        
        std::vector<Bitboard> bishop_blockers_bitboards = all_blockers_bitboards(current_bishop_blocker_possibilities_bitboard);
        std::vector<Bitboard> rook_blockers_bitboards   = all_blockers_bitboards(current_rook_blocker_possibilities_bitboard);

        std::vector<Bitboard> corresponding_bishop_attacks_bitboards;
        std::vector<Bitboard> corresponding_rook_attacks_bitboards;
        corresponding_bishop_attacks_bitboards.reserve(bishop_blockers_bitboards.size());
        corresponding_rook_attacks_bitboards  .reserve(rook_blockers_bitboards  .size());

        for (Bitboard blockers_bitboard:bishop_blockers_bitboards)
        {
            corresponding_bishop_attacks_bitboards.push_back(slider_attacks_bitboard(square, blockers_bitboard, bishop_directions));
        }
        for (Bitboard blockers_bitboard:rook_blockers_bitboards)
        {
            corresponding_rook_attacks_bitboards.push_back(slider_attacks_bitboard(square, blockers_bitboard, rook_directions));
        }

        unsigned int bishop_bits = popcount_64(current_bishop_blocker_possibilities_bitboard);
        unsigned int rook_bits   = popcount_64(current_rook_blocker_possibilities_bitboard);

        bishop_shifts[square] = 64 - bishop_bits;
        rook_shifts  [square] = 64 - rook_bits;

        MagicData bishop_magic_data = find_magic(bishop_blockers_bitboards, corresponding_bishop_attacks_bitboards);
        MagicData rook_magic_data   = find_magic(rook_blockers_bitboards,   corresponding_rook_attacks_bitboards);
        
        bishop_magic_bitboards[square] = bishop_magic_data.magic;
        rook_magic_bitboards  [square] = rook_magic_data  .magic;
        
        append_vector(ordered_bishop_attacks_bitboards, bishop_magic_data.attacks_bitboards);
        append_vector(ordered_rook_attacks_bitboards,   rook_magic_data  .attacks_bitboards);

        ordered_bishop_attacks_bitboards_row_offsets[square] = bishop_magic_data.attacks_bitboards.size(); // 0 somehow
        ordered_rook_attacks_bitboards_row_offsets  [square] = rook_magic_data  .attacks_bitboards.size(); // also 0
        if (square > 0)
        {
            ordered_bishop_attacks_bitboards_row_offsets[square] += ordered_bishop_attacks_bitboards_row_offsets[square - 1];
            ordered_rook_attacks_bitboards_row_offsets  [square] += ordered_rook_attacks_bitboards_row_offsets  [square - 1];
        }

        std::cout << "square " << (int)square << " done\n";
    }

    std::ofstream out{"src/precomputed_bitboards.h"};
    out << "#pragma once\n\n#include <vector>\n\n#include <cstddef>\n#include <cstdint>\n\n";

    dump_array(out, "knight_attacks_bitboards",             "std::uint64_t", "ULL", knight_attacks_bitboards);
    dump_array(out, "king_attacks_bitboards",               "std::uint64_t", "ULL", king_attacks_bitboards);
    dump_array(out, "bishop_magics",                        "std::uint64_t", "ULL", bishop_magic_bitboards);
    dump_array(out, "rook_magics",                          "std::uint64_t", "ULL", rook_magic_bitboards);
    dump_array(out, "bishop_shifts",                        "unsigned int",  "U",   bishop_shifts);
    dump_array(out, "rook_shifts",                          "unsigned int",  "U",   rook_shifts);
    dump_array(out, "bishop_attacks_bitboards",             "std::uint64_t", "ULL", ordered_bishop_attacks_bitboards);
    dump_array(out, "rook_attacks_bitboards",               "std::uint64_t", "ULL", ordered_rook_attacks_bitboards);
    dump_array(out, "bishop_attacks_bitboards_row_offsets", "std::size_t",   "U",   ordered_bishop_attacks_bitboards_row_offsets);
    dump_array(out, "rook_attacks_bitboards_row_offsets",   "std::size_t",   "U",   ordered_rook_attacks_bitboards_row_offsets);

    std::cout << "everything is done. written to src/precomputed_bitboards.h.";
}