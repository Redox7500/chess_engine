#include <cstdint>
#include <array>
#include <vector>
#include <bitset>
#include <random>
#include <fstream>
#include <iostream>

using Bitboard = std::uint64_t;
using Square   = std::uint8_t;
using Coord    = std::uint8_t;

constexpr int popcount_64(Bitboard x) {return __builtin_popcountll(x);}
constexpr Square lsb_64(Bitboard x) {return __builtin_ctzll(x);}

constexpr Coord file_from_square(Square square) {return square & 0b000111;}
constexpr Coord rank_from_square(Square square) {return square >> 3;}
constexpr Bitboard bitboard_from_square(Square square) {return 1 << square;}
constexpr Square square_from_coords(Coord file, Coord rank) {return rank * 8 + file;}

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

Bitboard white_pawn_pushes_bitboard(Square square)
{
    Bitboard attacks_bitboard = 0;

    const Coord file = file_from_square(square);
    const Coord rank = rank_from_square(square);

    if (rank < 7)
    {
        attacks_bitboard |= bitboard_from_square(square_from_coords(file, rank + 1));
        if (rank == 1) {attacks_bitboard |= bitboard_from_square(square_from_coords(file, 3));}
    }

    return attacks_bitboard;
}

Bitboard black_pawn_pushes_bitboard(Square square)
{
    Bitboard attacks_bitboard = 0;

    const Coord file = file_from_square(square);
    const Coord rank = rank_from_square(square);

    if (rank > 0)
    {
        attacks_bitboard |= bitboard_from_square(square_from_coords(file, rank - 1));
        if (rank == 6) {attacks_bitboard |= bitboard_from_square(square_from_coords(file, 4));}
    }

    return attacks_bitboard;
}

Bitboard white_pawn_attacks_bitboard(Square square)
{
    Bitboard attacks_bitboard = 0;

    const Coord file = file_from_square(square);
    const Coord rank = rank_from_square(square);

    if (rank < 7)
    {
        if (file > 0) {attacks_bitboard |= bitboard_from_square(square_from_coords(file - 1, rank + 1));}
        if (file < 7) {attacks_bitboard |= bitboard_from_square(square_from_coords(file + 1, rank + 1));}
    }

    return attacks_bitboard;
}

Bitboard black_pawn_attacks_bitboard(Square square)
{
    Bitboard attacks_bitboard = 0;

    const Coord file = file_from_square(square);
    const Coord rank = rank_from_square(square);

    if (rank > 0)
    {
        if (file > 0) {attacks_bitboard |= bitboard_from_square(square_from_coords(file - 1, rank - 1));}
        if (file < 7) {attacks_bitboard |= bitboard_from_square(square_from_coords(file + 1, rank - 1));}
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

Bitboard bishop_blocker_possibilities_bitboard(Square square)
{
    Bitboard attacks_bitboard = 0;

    const Coord file = file_from_square(square);
    const Coord rank = rank_from_square(square);

    for (Coord i = 1; file > i     && rank > i;     i++) {attacks_bitboard |= bitboard_from_square(square_from_coords(file - i, rank - i));}
    for (Coord i = 1; file > i     && rank + i < 7; i++) {attacks_bitboard |= bitboard_from_square(square_from_coords(file - i, rank + i));}
    for (Coord i = 1; file + i < 7 && rank > i;     i++) {attacks_bitboard |= bitboard_from_square(square_from_coords(file + i, rank - i));}
    for (Coord i = 1; file + i < 7 && rank + i < 7; i++) {attacks_bitboard |= bitboard_from_square(square_from_coords(file + i, rank + i));}

    return attacks_bitboard;
}

Bitboard rook_blocker_possibilities_bitboard(Square square)
{
    Bitboard attacks_bitboard = 0;

    const Coord file = file_from_square(square);
    const Coord rank = rank_from_square(square);

    for (Coord i = 1;        i < file; i++) {attacks_bitboard |= bitboard_from_square(square_from_coords(i, rank));}
    for (Coord i = file + 1; i < 7;    i++) {attacks_bitboard |= bitboard_from_square(square_from_coords(i, rank));}
    for (Coord i = 1;        i < rank; i++) {attacks_bitboard |= bitboard_from_square(square_from_coords(file, i));}
    for (Coord i = rank + 1; i < 7;    i++) {attacks_bitboard |= bitboard_from_square(square_from_coords(file, i));}

    return attacks_bitboard;
}

Bitboard bishop_attacks_bitboard(Square square, Bitboard blockers_bitboard)
{
    Bitboard attacks_bitboard = 0;

    const Coord file = file_from_square(square);
    const Coord rank = rank_from_square(square);

    // const std::array<std::array<int, 2>, 4> directions = {{
    //     {{-1, -1}},
    //     {{-1,  1}},
    //     {{ 1, -1}},
    //     {{ 1,  1}}
    // }};

    // for (const std::array<int, 2> direction:directions)
    // {
        
    // }

    for (Coord i = 1; file >= i && rank >= i; i++)
    {
        const Bitboard bit = bitboard_from_square(square_from_coords(file - i, rank - i));
        attacks_bitboard |= bit;

        if (blockers_bitboard & bit) {break;}
    }
    for (Coord i = 1; file >= i && rank + i < 8; i++)
    {
        const Bitboard bit = bitboard_from_square(square_from_coords(file - i, rank + i));
        attacks_bitboard |= bit;

        if (blockers_bitboard & bit) {break;}
    }
    for (Coord i = 1; file + i < 8 && rank >= i; i++)
    {
        const Bitboard bit = bitboard_from_square(square_from_coords(file + i, rank - i));
        attacks_bitboard |= bit;

        if (blockers_bitboard & bit) {break;}
    }
    for (Coord i = 1; file + i < 8 && rank + i < 8; i++)
    {
        const Bitboard bit = bitboard_from_square(square_from_coords(file + i, rank + i));
        attacks_bitboard |= bit;

        if (blockers_bitboard & bit) {break;}
    }

    print_bitboard(attacks_bitboard);
    std::cout << "\n";
    return attacks_bitboard;
}

Bitboard rook_attacks_bitboard(Square square, Bitboard blockers_bitboard)
{
    Bitboard attacks_bitboard = 0;

    const Coord file = file_from_square(square);
    const Coord rank = rank_from_square(square);

    for (Coord i = file - 1; i + 1 > 0; i--)
    {
        const Bitboard bit = bitboard_from_square(square_from_coords(i, rank));
        attacks_bitboard |= bit;

        if (blockers_bitboard & bit) {break;}
    }
    for (Coord i = file + 1; i < 8; i++)
    {
        const Bitboard bit = bitboard_from_square(square_from_coords(i, rank));
        attacks_bitboard |= bit;

        if (blockers_bitboard & bit) {break;}
    }
    for (Coord i = rank - 1; i + 1 > 0; i--)
    {
        const Bitboard bit = bitboard_from_square(square_from_coords(file, i));
        attacks_bitboard |= bit;

        if (blockers_bitboard & bit) {break;}
    }
    for (Coord i = rank + 1; i < 8; i++)
    {
        const Bitboard bit = bitboard_from_square(square_from_coords(file, i));
        attacks_bitboard |= bit;

        if (blockers_bitboard & bit) {break;}
    }

    return attacks_bitboard;
}

// U64 nth_subset(U64 mask, int index)
// {
//     U64 subset = 0;
//     int bit_index = 0;

//     while (mask)
//     {
//         int square = __builtin_ctzll(mask);
//         mask &= mask - 1;

//         if (index & bitboard_from_square(bit_index))
//         {
//             subset |= bitboard_from_square(square);
//         }

//         bit_index++;
//     }

//     return subset;
// }

std::vector<Bitboard> all_blockers_bitboards(Bitboard naive_attacks_bitboard)
{
    const int bits = popcount_64(naive_attacks_bitboard);
    const std::size_t size = 1 << bits;

    std::vector<Bitboard> blocker_bitboards(size);
    Bitboard blocker_bitboard = naive_attacks_bitboard;
    for (std::size_t i = 0; i < size && blocker_bitboard > 0; i++, blocker_bitboard = (blocker_bitboard - 1) & naive_attacks_bitboard)
    {
        blocker_bitboards.push_back(blocker_bitboard);
    }

    return blocker_bitboards;
}

Bitboard random_magic(std::mt19937_64& rng)
{
    return rng() & rng();
}

struct MagicData
{
    Bitboard magic;
    std::vector<Bitboard> attacks_bitboards;
};

MagicData find_magic(const std::vector<Bitboard>& blockers_bitboards, const std::vector<Bitboard>& attacks_bitboards, int bits)
{
    if (blockers_bitboards.size() != attacks_bitboards.size())
    {
        std::cerr << "Mismatched blockers bitboards and attacks bitboards; each blockers bitboard should match with each attacks bitboard 1:1\n";
        return {};
    }

    const std::size_t size = 1 << bits;

    if (blockers_bitboards.size() > size)
    {
        std::cerr << "More bitboards than bits available for indices!\n";
        return {};
    }

    std::vector<Bitboard> used(size);

    std::mt19937_64 rng(0x511);
    while (true)
    {
        Bitboard magic = random_magic(rng);

        // if (__builtin_popcountll((magic * mask) >> (64 - bits)) < 6)
        // {
        //     continue;
        // }

        std::fill(used.begin(), used.end(), 0);

        bool fail = false;
        for (std::size_t i = 0; i < size; i++)
        {
            // print_bitboard(blockers_bitboards[i]);
            // std::cout << "\n";
            std::size_t index = (blockers_bitboards[i] * magic) >> (64 - bits);
            if (index >= size || used[index] != attacks_bitboards[i])
            {
                fail = true;
                break;
            }
            if (!used[index])
            {
                used[index] = attacks_bitboards[i];
            }
        }

        if (!fail)
        {
            return {magic, used};
        }
    }
}

template <typename T, std::size_t L>
void dump_array(std::ofstream& out, const char* name, const char* datatype, const char* suffix, const std::array<T, L>& array)
{
    std::size_t size = array.size();
    out << "constexpr " << datatype << " " << name << "[" << size << "] = {";
    if (size)
    {
        out << "\n";
        for (std::size_t i = 0; i < size; i++)
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

    const std::size_t vector_size = vector.size();
    if (vector_size)
    {
        out << "\n";
        for (std::size_t i = 0; i < vector_size; i++)
        {
            const std::size_t inner_vector_size = vector[i].size();

            out << "    {\n";
            for (std::size_t j = 0; j < inner_vector_size - 1; j++)
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
    std::array<Bitboard, 64> knight_attacks_bitboards, king_attacks_bitboards;
    std::vector<std::vector<Bitboard>> ordered_bishop_attacks_bitboards(64), ordered_rook_attacks_bitboards(64);
    std::array<Bitboard, 64> bishop_magic_bitboards, rook_magic_bitboards;
    std::array<int, 64> rook_shifts, bishop_shifts;

    for (Square square = 0; square < 64; square++)
    {
        knight_attacks_bitboards[square] = knight_attacks_bitboard(square);
        king_attacks_bitboards  [square] = king_attacks_bitboard  (square);

        const Bitboard current_bishop_blocker_possibilities_bitboard = bishop_blocker_possibilities_bitboard(square);
        const Bitboard current_rook_blocker_possibilities_bitboard   = rook_blocker_possibilities_bitboard  (square);
        std::vector<Bitboard> corresponding_bishop_attacks_bitboards;
        std::vector<Bitboard> corresponding_rook_attacks_bitboards;
        
        std::vector<Bitboard> bishop_blockers_bitboards = all_blockers_bitboards(current_bishop_blocker_possibilities_bitboard);
        std::vector<Bitboard> rook_blockers_bitboards   = all_blockers_bitboards(current_rook_blocker_possibilities_bitboard);

        for (Bitboard blockers_bitboard:bishop_blockers_bitboards)
        {
            corresponding_bishop_attacks_bitboards.push_back(bishop_attacks_bitboard(square, blockers_bitboard));
        }
        for (Bitboard blockers_bitboard:rook_blockers_bitboards)
        {
            corresponding_rook_attacks_bitboards.push_back(rook_attacks_bitboard(square, blockers_bitboard));
        }

        int bishop_bits = popcount_64(current_bishop_blocker_possibilities_bitboard);
        int rook_bits   = popcount_64(current_rook_blocker_possibilities_bitboard);

        bishop_shifts[square] = 64 - bishop_bits;
        rook_shifts  [square] = 64 - rook_bits;

        const MagicData bishop_magic_data = find_magic(bishop_blockers_bitboards, corresponding_bishop_attacks_bitboards, bishop_bits);
        const MagicData rook_magic_data   = find_magic(rook_blockers_bitboards  , corresponding_rook_attacks_bitboards  , rook_bits);
        bishop_magic_bitboards[square] = bishop_magic_data.magic;
        rook_magic_bitboards  [square] = rook_magic_data  .magic;
        ordered_bishop_attacks_bitboards[square] = bishop_magic_data.attacks_bitboards;
        ordered_rook_attacks_bitboards  [square] = rook_magic_data  .attacks_bitboards;

        std::cout << "Square " << (int)square << " done\n";
    }

    std::ofstream out("precomputed_bitboards.h");
    out << "#pragma once\n#include <cstdint>\n\nusing U64 = std::uint64_t;\n\n";

    dump_array(out, "knight_attacks_bitboards", "U64"         , "ULL", knight_attacks_bitboards);
    dump_array(out, "king_attacks_bitboards"  , "U64"         , "ULL", king_attacks_bitboards  );
    dump_array(out, "bishop_magics"           , "U64"         , "ULL", bishop_magic_bitboards  );
    dump_array(out, "rook_magics"             , "U64"         , "ULL", rook_magic_bitboards    );
    dump_array(out, "bishop_shifts"           , "unsigned int", "U"  , bishop_shifts           );
    dump_array(out, "rook_shifts"             , "unsigned int", "U"  , rook_shifts             );
    dump_nested_vector(out, "bishop_attacks_bitboards", "U64", "ULL", ordered_bishop_attacks_bitboards);
    dump_nested_vector(out, "rook_attacks_bitboards"  , "U64", "ULL", ordered_rook_attacks_bitboards  );

    std::cout << "Done. precomputed_bitboards.h written";
}