#include <cstdint>
#include <array>
#include <vector>
#include <bitset>
#include <random>
#include <fstream>
#include <iostream>

using U8 = std::uint8_t;
using U16 = std::uint16_t;
using U64 = std::uint64_t;

constexpr U8 popcount_64(U64 x) {return __builtin_popcountll(x);}
constexpr U8 lsb_64(U64 x) {return __builtin_ctzll(x);}

constexpr U8 file_from_square(U8 square) {return square & 0b000111;}
constexpr U8 rank_from_square(U8 square) {return square >> 3;}
constexpr U64 bit_from_square(U8 square) {return U64(1) << square;}
constexpr U8 to_square(U8 file, U8 rank) {return rank * 8 + file;}

void print_bitboard(U64 bitboard)
{
    for (U8 rank = 8; rank-- > 0; )
    {
        for (U8 file = 0; file < 8; file++)
        {
            std::cout << !!(bitboard & bit_from_square(to_square(file, rank))) << " ";
        }
        std::cout << "\n";
    }
}

U64 king_attacks_bitboard(U8 square)
{
    U64 attacks_bitboard = 0;

    const U8 file = file_from_square(square);
    const U8 rank = rank_from_square(square);

    attacks_bitboard |= bit_from_square(to_square(file, rank - 1));
    attacks_bitboard |= bit_from_square(to_square(file, rank + 1));
    if (file > 0)
    {
        attacks_bitboard |= bit_from_square(to_square(file - 1, rank));
        if (rank > 0) {attacks_bitboard |= bit_from_square(to_square(file - 1, rank - 1));}
        if (rank < 7) {attacks_bitboard |= bit_from_square(to_square(file - 1, rank + 1));}
    }
    if (file < 7)
    {
        attacks_bitboard |= bit_from_square(to_square(file + 1, rank));
        if (rank > 0) {attacks_bitboard |= bit_from_square(to_square(file + 1, rank - 1));}
        if (rank < 7) {attacks_bitboard |= bit_from_square(to_square(file + 1, rank + 1));}
    }

    return attacks_bitboard;
}

U64 white_pawn_pushes_bitboard(U8 square)
{
    U64 attacks_bitboard = 0;

    const U8 file = file_from_square(square);
    const U8 rank = rank_from_square(square);

    if (rank < 7)
    {
        attacks_bitboard |= bit_from_square(to_square(file, rank + 1));
        if (rank == 1) {attacks_bitboard |= bit_from_square(to_square(file, 3));}
    }

    return attacks_bitboard;
}

U64 black_pawn_pushes_bitboard(U8 square)
{
    U64 attacks_bitboard = 0;

    const U8 file = file_from_square(square);
    const U8 rank = rank_from_square(square);

    if (rank > 0)
    {
        attacks_bitboard |= bit_from_square(to_square(file, rank - 1));
        if (rank == 6) {attacks_bitboard |= bit_from_square(to_square(file, 4));}
    }

    return attacks_bitboard;
}

U64 white_pawn_attacks_bitboard(U8 square)
{
    U64 attacks_bitboard = 0;

    const U8 file = file_from_square(square);
    const U8 rank = rank_from_square(square);

    if (rank < 7)
    {
        if (file > 0) {attacks_bitboard |= bit_from_square(to_square(file - 1, rank + 1));}
        if (file < 7) {attacks_bitboard |= bit_from_square(to_square(file + 1, rank + 1));}
    }

    return attacks_bitboard;
}

U64 black_pawn_attacks_bitboard(U8 square)
{
    U64 attacks_bitboard = 0;

    const U8 file = file_from_square(square);
    const U8 rank = rank_from_square(square);

    if (rank > 0)
    {
        if (file > 0) {attacks_bitboard |= bit_from_square(to_square(file - 1, rank - 1));}
        if (file < 7) {attacks_bitboard |= bit_from_square(to_square(file + 1, rank - 1));}
    }

    return attacks_bitboard;
}

U64 knight_attacks_bitboard(U8 square)
{
    U64 attacks_bitboard = 0;

    const U8 file = file_from_square(square);
    const U8 rank = rank_from_square(square);

    if (file > 0)
    {
        if (rank > 1) {attacks_bitboard |= bit_from_square(to_square(file - 1, rank - 2));}
        if (rank < 6) {attacks_bitboard |= bit_from_square(to_square(file - 1, rank + 2));}

        if (file > 1)
        {
            if (rank > 0) {attacks_bitboard |= bit_from_square(to_square(file - 2, rank - 1));}
            if (rank < 7) {attacks_bitboard |= bit_from_square(to_square(file - 2, rank + 1));}
        }
    }
    if (file < 7)
    {
        if (rank > 1) {attacks_bitboard |= bit_from_square(to_square(file + 1, rank - 2));}
        if (rank < 6) {attacks_bitboard |= bit_from_square(to_square(file + 1, rank + 2));}

        if (file < 6)
        {
            if (rank > 0) {attacks_bitboard |= bit_from_square(to_square(file + 2, rank - 1));}
            if (rank < 7) {attacks_bitboard |= bit_from_square(to_square(file + 2, rank + 1));}
        }
    }

    return attacks_bitboard;
}

U64 bishop_attacks_bitboard(U8 square, U64 blockers)
{
    U64 attacks_bitboard = 0;

    const U8 file = file_from_square(square);
    const U8 rank = rank_from_square(square);

    for (U8 i = 1; file > i && rank > i; i++)
    {
        const U8 bit = bit_from_square(to_square(file - i, rank - i));
        attacks_bitboard |= bit;

        if (blockers & bit) {break;}
    }
    for (U8 i = 1; file > i && rank + i < 7; i++)
    {
        const U8 bit = bit_from_square(to_square(file - i, rank + i));
        attacks_bitboard |= bit;

        if (blockers & bit) {break;}
    }
    for (U8 i = 1; file + i < 7 && rank > i; i++)
    {
        const U8 bit = bit_from_square(to_square(file + i, rank - i));
        attacks_bitboard |= bit;

        if (blockers & bit) {break;}
    }
    for (U8 i = 1; file + i < 7 && rank + i < 7; i++)
    {
        const U8 bit = bit_from_square(to_square(file + i, rank + i));
        attacks_bitboard |= bit;

        if (blockers & bit) {break;}
    }

    return attacks_bitboard;
}

U64 rook_attacks_bitboard(U8 square, U64 blockers)
{
    U64 attacks_bitboard = 0;

    const U8 file = file_from_square(square);
    const U8 rank = rank_from_square(square);

    for (U8 i = 1; i < file; i++)
    {
        const U8 bit = bit_from_square(to_square(i, rank));
        attacks_bitboard |= bit;

        if (blockers & bit) {break;}
    }
    for (U8 i = file + 1; i < 7; i++)
    {
        const U8 bit = bit_from_square(to_square(i, rank));
        attacks_bitboard |= bit;

        if (blockers & bit) {break;}
    }
    for (U8 i = 1; i < rank; i++)
    {
        const U8 bit = bit_from_square(to_square(file, i));
        attacks_bitboard |= bit;

        if (blockers & bit) {break;}
    }
    for (U8 i = rank + 1; i < 7; i++)
    {
        const U8 bit = bit_from_square(to_square(file, i));
        attacks_bitboard |= bit;

        if (blockers & bit) {break;}
    }

    return attacks_bitboard;
}

U64 bishop_blocker_possibilities_bitboard(U8 square)
{
    U64 attacks_bitboard = 0;

    const U8 file = file_from_square(square);
    const U8 rank = rank_from_square(square);

    for (U8 i = 1; file > i     && rank > i;     i++) {attacks_bitboard |= bit_from_square(to_square(file - i, rank - i));}
    for (U8 i = 1; file > i     && rank + i < 7; i++) {attacks_bitboard |= bit_from_square(to_square(file - i, rank + i));}
    for (U8 i = 1; file + i < 7 && rank > i;     i++) {attacks_bitboard |= bit_from_square(to_square(file + i, rank - i));}
    for (U8 i = 1; file + i < 7 && rank + i < 7; i++) {attacks_bitboard |= bit_from_square(to_square(file + i, rank + i));}

    return attacks_bitboard;
}

U64 rook_blocker_possibilities_bitboard(U8 square)
{
    U64 attacks_bitboard = 0;

    const U8 file = file_from_square(square);
    const U8 rank = rank_from_square(square);

    for (U8 i = 1;        i < file; i++) {attacks_bitboard |= bit_from_square(to_square(i, rank));}
    for (U8 i = file + 1; i < 7;    i++) {attacks_bitboard |= bit_from_square(to_square(i, rank));}
    for (U8 i = 1;        i < rank; i++) {attacks_bitboard |= bit_from_square(to_square(file, i));}
    for (U8 i = rank + 1; i < 7;    i++) {attacks_bitboard |= bit_from_square(to_square(file, i));}

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

//         if (index & bit_from_square(bit_index))
//         {
//             subset |= bit_from_square(square);
//         }

//         bit_index++;
//     }

//     return subset;
// }

std::vector<U64> all_blocker_bitboards(U64 naive_attacks_bitboard)
{
    const U8 bits = popcount_64(naive_attacks_bitboard);
    const U16 size = 1 << bits;

    std::vector<U64> blocker_bitboards(size);
    U64 blocker_bitboard = naive_attacks_bitboard;
    for (U16 i = 0; i < size && blocker_bitboard > 0; i++, blocker_bitboard = (blocker_bitboard - 1) & naive_attacks_bitboard)
    {
        blocker_bitboards.push_back(blocker_bitboard);
    }

    return blocker_bitboards;
}

U64 random_magic(std::mt19937_64& rng)
{
    return rng() & rng();
}

struct magic_data
{
    U64 magic;
    std::vector<U64> attacks_bitboards;
};

magic_data find_magic(const std::vector<U64>& blockers_bitboards, const std::vector<U64>& attacks_bitboards, U8 bits)
{
    if (blockers_bitboards.size() != attacks_bitboards.size())
    {
        std::cerr << "Mismatched blockers bitboards and attacks bitboards; each blockers bitboard should match with each attacks bitboard 1:1\n";
        return {};
    }

    const U16 size = 1 << bits;

    if (blockers_bitboards.size() > size)
    {
        std::cerr << "More bitboards than bits available for indices!\n";
        return {};
    }

    std::vector<U64> used(size);

    std::mt19937_64 rng(0x511);
    while (true)
    {
        U64 magic = random_magic(rng);

        // if (__builtin_popcountll((magic * mask) >> (64 - bits)) < 6)
        // {
        //     continue;
        // }

        std::fill(used.begin(), used.end(), 0);

        bool fail = false;
        for (U16 i = 0; i < size; i++)
        {
            print_bitboard(blockers_bitboards[i]);
            std::cout << "\n";
            U16 index = (blockers_bitboards[i] * magic) >> (64 - bits);
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
    std::array<U64, 64> knight_attacks_bitboards, king_attacks_bitboards;
    std::vector<std::vector<U64>> ordered_bishop_attacks_bitboards(64), ordered_rook_attacks_bitboards(64);
    std::array<U64, 64> bishop_magic_bitboards, rook_magic_bitboards;
    std::array<U8, 64> rook_shifts, bishop_shifts;

    for (U8 square = 0; square < 64; square++)
    {
        knight_attacks_bitboards[square] = knight_attacks_bitboard(square);
        king_attacks_bitboards[square] = king_attacks_bitboard(square);

        const U64 current_bishop_blocker_possibilities_bitboard = bishop_blocker_possibilities_bitboard(square);
        const U64 current_rook_blocker_possibilities_bitboard = rook_blocker_possibilities_bitboard(square);
        std::vector<U64> corresponding_bishop_attacks_bitboards;
        std::vector<U64> corresponding_rook_attacks_bitboards;
        
        std::vector<U64> bishop_blocker_bitboards = all_blocker_bitboards(current_bishop_blocker_possibilities_bitboard);
        std::vector<U64> rook_blocker_bitboards = all_blocker_bitboards(current_rook_blocker_possibilities_bitboard);

        for (U64 blocker_bitboard:bishop_blocker_bitboards)
        {
            corresponding_bishop_attacks_bitboards.push_back(bishop_attacks_bitboard(square, blocker_bitboard));
        }
        for (U64 blocker_bitboard:rook_blocker_bitboards)
        {
            corresponding_rook_attacks_bitboards.push_back(rook_attacks_bitboard(square, blocker_bitboard));
        }

        U8 bishop_bits = popcount_64(current_bishop_blocker_possibilities_bitboard);
        U8 rook_bits = popcount_64(current_rook_blocker_possibilities_bitboard);

        bishop_shifts[square] = 64 - bishop_bits;
        rook_shifts[square] = 64 - rook_bits;

        bishop_magic_bitboards[square] = find_magic(bishop_blocker_bitboards, corresponding_bishop_attacks_bitboards, bishop_bits).magic;
        rook_magic_bitboards[square] = find_magic(rook_blocker_bitboards, corresponding_rook_attacks_bitboards, rook_bits).magic;

        std::cout << "Square " << square << " done\n";
    }

    std::ofstream out("precomputed_bitboards.h");
    out << "#pragma once\n#include <cstdint>\n\n";

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