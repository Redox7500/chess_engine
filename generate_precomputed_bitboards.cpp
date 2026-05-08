#include <cstdint>
#include <vector>
#include <bitset>
#include <random>
#include <fstream>
#include <iostream>

// maybe start using stuff like int8_t and size_t?

inline int popcount_64(uint64_t x) {return __builtin_popcountll(x);}
inline int lsb_64(uint64_t x) {return __builtin_ctzll(x);}

int file_from_square(int square) {return square & 0b000111;}
int rank_from_square(int square) {return square >> 3;}
uint64_t bit_from_square(int square) {return uint64_t(1) << square;}
int to_square(int file, int rank) {return rank * 8 + file;}

void print_bitboard(uint64_t bitboard)
{
    for (int rank = 7; rank >= 0; rank--)
    {
        for (int file = 0; file < 8; file++)
        {
            std::cout << !!(bitboard & bit_from_square(to_square(file, rank))) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

uint64_t king_attack_mask(int square)
{
    uint64_t mask = 0;

    const int file = file_from_square(square);
    const int rank = rank_from_square(square);

    mask |= bit_from_square(to_square(file - 1, rank - 1));
    mask |= bit_from_square(to_square(file - 1, rank));
    mask |= bit_from_square(to_square(file - 1, rank + 1));
    mask |= bit_from_square(to_square(file, rank - 1));
    mask |= bit_from_square(to_square(file, rank + 1));
    mask |= bit_from_square(to_square(file + 1, rank - 1));
    mask |= bit_from_square(to_square(file + 1, rank));
    mask |= bit_from_square(to_square(file + 1, rank + 1));

    return mask;
}

uint64_t white_pawn_push_mask(int square)
{
    uint64_t mask = 0;

    const int file = file_from_square(square);
    const int rank = rank_from_square(square);

    if (rank < 7)
    {
        mask |= bit_from_square(to_square(file, rank + 1));
        if (rank == 1)
        {
            mask |= bit_from_square(to_square(file, 3));
        }
    }

    return mask;
}

uint64_t white_pawn_attack_mask(int square)
{
    uint64_t mask = 0;

    const int file = file_from_square(square);
    const int rank = rank_from_square(square);

    if (rank < 7)
    {
        if (file > 0)
        {
            mask |= bit_from_square(to_square(file - 1, rank + 1));
        }
        if (file < 7)
        {
            mask |= bit_from_square(to_square(file + 1, rank + 1));
        }
    }

    return mask;
}

uint64_t black_pawn_push_mask(int square)
{
    uint64_t mask = 0;

    const int file = file_from_square(square);
    const int rank = rank_from_square(square);

    if (rank > 0)
    {
        mask |= bit_from_square(to_square(file, rank - 1));
        if (rank == 6)
        {
            mask |= bit_from_square(to_square(file, 4));
        }
    }

    return mask;
}

uint64_t black_pawn_attack_mask(int square)
{
    uint64_t mask = 0;

    const int file = file_from_square(square);
    const int rank = rank_from_square(square);

    if (rank > 0)
    {
        if (file > 0)
        {
            mask |= bit_from_square(to_square(file - 1, rank - 1));
        }
        if (file < 7)
        {
            mask |= bit_from_square(to_square(file + 1, rank - 1));
        }
    }

    return mask;
}

uint64_t knight_attack_mask(int square)
{
    uint64_t mask = 0;

    const int file = file_from_square(square);
    const int rank = rank_from_square(square);

    mask |= bit_from_square(to_square(file - 2, rank - 1));
    mask |= bit_from_square(to_square(file - 2, rank + 1));
    mask |= bit_from_square(to_square(file - 1, rank - 2));
    mask |= bit_from_square(to_square(file - 1, rank + 2));
    mask |= bit_from_square(to_square(file + 1, rank - 2));
    mask |= bit_from_square(to_square(file + 1, rank + 2));
    mask |= bit_from_square(to_square(file + 2, rank - 1));
    mask |= bit_from_square(to_square(file + 2, rank + 1));

    return mask;
}

uint64_t bishop_attack_mask(int square, uint64_t blockers)
{
    uint64_t attacks = 0;

    const int file = file_from_square(square);
    const int rank = rank_from_square(square);

    for (int i = 1; file - i > 0 && rank - i > 0; i++)
    {
        const int bit = bit_from_square(to_square(file - i, rank - i));
        attacks |= bit;

        if (blockers & bit)
        {
            break;
        }
    }
    for (int i = 1; file + i < 7 && rank - i > 0; i++)
    {
        const int bit = bit_from_square(to_square(file - i, rank + i));
        attacks |= bit;

        if (blockers & bit)
        {
            break;
        }
    }
    for (int i = 1; file - i > 0 && rank + i < 7; i++)
    {
        const int bit = bit_from_square(to_square(file + i, rank - i));
        attacks |= bit;

        if (blockers & bit)
        {
            break;
        }
    }
    for (int i = 1; file + i < 7 && rank + i < 7; i++)
    {
        const int bit = bit_from_square(to_square(file + i, rank + i));
        attacks |= bit;

        if (blockers & bit)
        {
            break;
        }
    }

    return attacks;
}

uint64_t rook_attack_mask(int square, uint64_t blockers)
{
    uint64_t attacks = 0;

    const int file = file_from_square(square);
    const int rank = rank_from_square(square);

    for (int i = 1; i < file; i++)
    {
        const int bit = bit_from_square(to_square(i, rank));
        attacks |= bit;

        if (blockers & bit)
        {
            break;
        }
    }
    for (int i = file + 1; i < 7; i++)
    {
        const int bit = bit_from_square(to_square(i, rank));
        attacks |= bit;

        if (blockers & bit)
        {
            break;
        }
    }
    for (int i = 1; i < rank; i++)
    {
        const int bit = bit_from_square(to_square(file, i));
        attacks |= bit;

        if (blockers & bit)
        {
            break;
        }
    }
    for (int i = rank + 1; i < 7; i++)
    {
        const int bit = bit_from_square(to_square(file, i));
        attacks |= bit;

        if (blockers & bit)
        {
            break;
        }
    }

    return attacks;
}

// std::vector<uint64_t> bishopattack_mask

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

uint64_t random_magic(std::mt19937_64& rng)
{
    return rng() & rng() & rng();
}

uint64_t bishop_naive_attack_mask(int square)
{
    uint64_t mask = 0;

    const int file = file_from_square(square);
    const int rank = rank_from_square(square);

    for (int i = 1; file - i > 0 && rank - i > 0; i++) {mask |= bit_from_square(to_square(file - i, rank - i));}
    for (int i = 1; file - i > 0 && rank + i < 7; i++) {mask |= bit_from_square(to_square(file - i, rank + i));}
    for (int i = 1; file + i < 7 && rank - i > 0; i++) {mask |= bit_from_square(to_square(file + i, rank - i));}
    for (int i = 1; file + i < 7 && rank + i < 7; i++) {mask |= bit_from_square(to_square(file + i, rank + i));}

    return mask;
}

uint64_t rook_naive_attack_mask(int square)
{
    uint64_t mask = 0;

    const int file = file_from_square(square);
    const int rank = rank_from_square(square);

    for (int i = 1;        i < file; i++) {mask |= bit_from_square(to_square(i, rank));}
    for (int i = file + 1; i < 7;    i++) {mask |= bit_from_square(to_square(i, rank));}
    for (int i = 1;        i < rank; i++) {mask |= bit_from_square(to_square(file, i));}
    for (int i = rank + 1; i < 7;    i++) {mask |= bit_from_square(to_square(file, i));}

    return mask;
}

std::vector<uint64_t> all_blocker_bitboards(uint64_t naive_attack_mask)
{
    int bits = popcount_64(naive_attack_mask);

    std::vector<uint64_t> blocker_bitboards(bits);
    uint64_t blocker_bitboard = 0;
    for (int i = 0; i < 1 << bits; i++)
    {
        blocker_bitboards.push_back(blocker_bitboard);

        blocker_bitboard = (blocker_bitboard - naive_attack_mask) & naive_attack_mask;
    }

    return blocker_bitboards;
}

// std::vector<uint64_t> bishop_attack_masks(std::vector<uint64_t> blocker_bitboards)
// {
//     std::vector<uint64_t> blocker_bitboards = all_blocker_bitboards(bishop_naive_attack_mask(square));
//     std::vector<uint64_t> attack_masks(blocker_bitboards.size());
//     for (uint64_t blocker_bitboard:blocker_bitboards)
//     {
//         attack_masks.push_back(bishop_attack_mask(square, blocker_bitboard));
//     }
//     return attack_masks;
// }

// std::vector<uint64_t> rook_attack_masks(int square)
// {
//     std::vector<uint64_t> blocker_bitboards = all_blocker_bitboards(rook_naive_attack_mask(square));
//     std::vector<uint64_t> attack_masks(blocker_bitboards.size());
//     for (uint64_t blocker_bitboard:blocker_bitboards)
//     {
//         attack_masks.push_back(rook_attack_mask(square, blocker_bitboard));
//     }
//     return attack_masks;
// }

uint64_t bishop_magic(int square, std::vector<uint64_t> blocker_bitboards, std::vector<uint64_t> attack_bitboards, int bits)
{
    // const int bits = popcount_64(bishop_naive_attack_mask(square)); // also popcount_64(attack_bitboards[-1]) probably
    const int size = 1 << bits;
    std::cout << size << std::endl;

    // std::vector<uint64_t> blocker_bitboards(size);
    // std::vector<uint64_t> attack_bitboards(size);
    std::vector<uint64_t> used(size);

    // uint64_t blocker_bitboard = 0;
    // for (int i = 0; i < size; i++)
    // {
    //     blocker_bitboard = (blocker_bitboard - naive_attack_mask) & naive_attack_mask;
        
    //     blocker_bitboards[i] = blocker_bitboard;
    //     attack_bitboards[i] = bishop_attack_mask(square, blocker_bitboard);
    // }

    std::mt19937_64 rng(square);
    while (true)
    {
        uint64_t magic = random_magic(rng);

        // if (__builtin_popcountll((magic * mask) >> (64 - bits)) < 6)
        // {
        //     continue;
        // }

        std::fill(used.begin(), used.end(), 0);

        bool fail = false;
        for (int i = 0; i < size; i++)
        {
            std::cout << bits << std::endl;
            int index = (blocker_bitboards[i] * magic) >> (64 - bits);
            if (!used[index])
            {
                used[index] = attack_bitboards[i];
            }
            else if (used[index] != attack_bitboards[i])
            {
                fail = true;
                break;
            }
        }

        if (!fail)
        {
            return magic;
        }
    }
}

uint64_t rook_magic(int square, std::vector<uint64_t> blocker_bitboards, std::vector<uint64_t> attack_bitboards, int bits)
{
    // const int bits = popcount_64(bishop_naive_attack_mask(square)); // also popcount_64(attack_bitboards[-1]) probably
    const int size = 1 << bits;

    // std::vector<uint64_t> blocker_bitboards(size);
    // std::vector<uint64_t> attack_bitboards(size);
    std::vector<uint64_t> used(size);

    // uint64_t blocker_bitboard = 0;
    // for (int i = 0; i < size; i++)
    // {
    //     blocker_bitboard = (blocker_bitboard - naive_attack_mask) & naive_attack_mask;
        
    //     blocker_bitboards[i] = blocker_bitboard;
    //     attack_bitboards[i] = rook_attack_mask(square, blocker_bitboard);
    // }

    std::mt19937_64 rng(square);
    while (true)
    {
        uint64_t magic = random_magic(rng);

        std::fill(used.begin(), used.end(), 0);

        bool fail = false;
        for (int i = 0; i < size; i++)
        {
            int index = (blocker_bitboards[i] * magic) >> (64 - bits);
            if (!used[index])
            {
                used[index] = attack_bitboards[i];
            }
            else if (used[index] != attack_bitboards[i])
            {
                fail = true;
                break;
            }
        }

        if (!fail)
        {
            return magic;
        }
    }
}

// uint64_t rook_magic(int square, uint64_t mask, int bits)
// {
//     const int size = 1 << bits;

//     std::vector<uint64_t> occupancies(size);
//     std::vector<uint64_t> attacks(size);
//     std::vector<uint64_t> used(size);

//     uint64_t occ = 0;
//     for (int i = 0; i < size; i++)
//     {
//         occupancies[i] = occ;
//         attacks[i] = rook_attack_mask(square, occ);
//     }

//     // int index = 0;
//     // for (uint64_t occ = 0; ; occ = (occ - mask) & mask)
//     // {
//     //     occupancies[index] = occ;
//     //     attacks[index] = rook_attack_mask(square, occ);

//     //     if (++index == size)
//     //     {
//     //         break;
//     //     }
//     // }

//     std::mt19937_64 rng(square);

//     while (true)
//     {
//         uint64_t magic = random_magic(rng);

//         // if (__builtin_popcountll((magic * mask) >> (64 - bits)) < 6)
//         // {
//         //     continue;
//         // }

//         std::fill(used.begin(), used.end(), 0);

//         bool fail = false;
//         for (int i = 0; i < size; i++)
//         {
//             int index = (occupancies[i] * magic) >> (64 - bits);
//             if (!used[index])
//             {
//                 used[index] = attacks[i];
//             }
//             else if (used[index] != attacks[i])
//             {
//                 fail = true;
//                 break;
//             }
//         }

//         if (!fail)
//         {
//             return magic;
//         }
//     }
// }

// uint64_t find_rook_magic(int square, int bits)
// {
//     std::mt19937_64 rng(0);

//     uint64_t mask = rook_mask(square);
//     int subset_count = 1 << bits;

//     std::vector<uint64_t> attacks(subset_count);
//     for (int i = 0; i < subset_count; i++)
//     {
//         uint64_t occ = nth_subset(mask, i);
//         attacks[i] = rook_attacks(square, occ);
//     }

//     while (true)
//     {
//         uint64_t magic = random_magic(rng);
//         std::fill(table.begin(), table.end(), std::numeric_limits<uint64_t>::max());
        
//         bool fail = false;
//         for (int i = 0; i < subset_count; i++)
//         {
//             uint64_t occ = nth_subset(mask, i);
//             int index = int(occ * magic >> 64 - bits);

//             if (table[index] == std::numeric_limits<uint64_t>::max())
//             {
//                 table[index] = attacks[i];
//             }
//             else if (table[index] != attacks[i])
//             {
//                 fail = true;
//                 break;
//             }
//         }

//         if (!fail)
//         {
//             return magic;
//         }
//     }
// }

template <typename T>
void dump_array(std::ofstream& out, const char* name, const char* datatype, const char* suffix, T* array, const int size)
{
    out << "constexpr " << datatype << " " << name << "[" << size << "] = {\n";
    for (int i = 0; i < size; i++)
    {
        out << "    " << array[i] << suffix << ",\n";
    }
    out << "    " << array[size - 1] << suffix << "\n};\n\n";
};

// template <typename T>
// void dump_array(std::ofstream& out, const char* name, const char* datatype, const char* suffix, std::vector<T> array)
// {
//     const int size = array.size();

//     out << "constexpr " << datatype << " " << name << "[" << size << "] = {\n";
//     for (int i = 0; i < size - 1; i++)
//     {
//         out << "    " << element << suffix << ",\n";
//     }
//     out << "    " << array[size - 1] << suffix << "\n};\n\n";
// };

template <typename T>
void dump_nested_vector(std::ofstream& out, const char* name, const char* datatype, const char* suffix, std::vector<std::vector<T>> vector)
{
    const int vector_size = vector.size();

    out << "constexpr std::vector<std::vector<" << datatype << ">> " << name << " = {\n";
    for (int i = 0; i < vector_size; i++)
    {
        const int inner_vector_size = vector[i].size();

        out << "    {\n";
        for (int j = 0; j < inner_vector_size - 1; j++)
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
    out << "};";
}

int main()
{
    uint64_t knight_attack_masks[64], king_attack_masks[64], bishop_naive_attack_masks[64], rook_naive_attack_masks[64], bishop_magics[64], rook_magics[64];
    std::vector<std::vector<uint64_t>> bishop_attack_masks(64), rook_attack_masks(64);
    unsigned int rook_shifts[64], bishop_shifts[64];

    for (int square = 0; square < 64; square++)
    {
        knight_attack_masks[square] = knight_attack_mask(square);
        king_attack_masks[square] = king_attack_mask(square);
        bishop_naive_attack_masks[square] = bishop_naive_attack_mask(square);
        rook_naive_attack_masks[square] = rook_naive_attack_mask(square);
        
        std::vector<uint64_t> bishop_blocker_bitboards = all_blocker_bitboards(bishop_naive_attack_mask(square));
        std::vector<uint64_t> rook_blocker_bitboards = all_blocker_bitboards(rook_naive_attack_mask(square));

        for (uint64_t blocker_bitboard:bishop_blocker_bitboards)
        {
            bishop_attack_masks[square].push_back(bishop_attack_mask(square, blocker_bitboard));
        }
        for (uint64_t blocker_bitboard:rook_blocker_bitboards)
        {
            bishop_attack_masks[square].push_back(rook_attack_mask(square, blocker_bitboard));
        }

        int bishop_bits = popcount_64(bishop_naive_attack_masks[square]);
        int rook_bits = popcount_64(rook_naive_attack_masks[square]);

        bishop_shifts[square] = 64 - bishop_bits;
        rook_shifts[square] = 64 - rook_bits;

        // bishop_magics[square] = bishop_magic(square, bishop_naive_attack_masks[square], bishop_bits);
        // rook_magics[square] = rook_magic(square, rook_naive_attack_masks[square], rook_bits);
        bishop_magics[square] = bishop_magic(square, bishop_blocker_bitboards, bishop_attack_masks[square], bishop_bits);
        rook_magics[square] = rook_magic(square, rook_blocker_bitboards, rook_attack_masks[square], rook_bits);

        std::cout << "Square " << square << " done\n";
    }

    std::ofstream out("precomputed_bitboards.h");
    out << "#pragma once\n#include <cstdint>\n\n";

    // auto dump_array = [&](const char* name, const char* datatype, const char* suffix, const auto& array)
    // {
    //     out << "constexpr " << datatype << " " << name << "[" << sizeof(array) / sizeof(array[0]) << "] = {\n";
    //     for (int i = 0; i < 63; i++)
    //     {
    //         out << "    " << array[i] << suffix << ",\n";
    //     }
    //     out << "    " << array[63] << suffix << "\n";
    //     out << "};\n\n";
    // };

    dump_array(out, "knight_attack_masks"      , "uint64_t"    , "ULL", knight_attack_masks      , 64);
    dump_array(out, "king_attack_masks"        , "uint64_t"    , "ULL", king_attack_masks        , 64);
    dump_array(out, "bishop_naive_attack_masks", "uint64_t"    , "ULL", bishop_naive_attack_masks, 64);
    dump_array(out, "rook_naive_attack_masks"  , "uint64_t"    , "ULL", rook_naive_attack_masks  , 64);
    dump_array(out, "bishop_magics"            , "uint64_t"    , "ULL", bishop_magics            , 64);
    dump_array(out, "rook_magics"              , "uint64_t"    , "ULL", rook_magics              , 64);
    dump_array(out, "bishop_shifts"            , "unsigned int", "U"  , bishop_shifts            , 64);
    dump_array(out, "rook_shifts"              , "unsigned int", "U"  , rook_shifts              , 64);
    dump_nested_vector(out, "bishop_attack_masks", "uint64_t", "ULL", bishop_attack_masks);
    dump_nested_vector(out, "rook_attack_masks"  , "uint64_t", "ULL", rook_attack_masks  );

    std::cout << "Done. precomputed_bitboards.h written";
}