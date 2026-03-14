#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

struct Token
{
    // true  = Là cặp (Distance, Length) -> Đây là dữ liệu đã nén
    // false = Là ký tự bình thường (Literal) -> Đây là dữ liệu chưa nén
    bool isMatch;

    char literal;

    int distance;
    int length;
};
#define lengthSize 256
#define matchSize 1280
struct Node
{
    int value;
    int freq;
    bool isMatch;
    Node *left;
    Node *right;

    Node(int value, int freq, bool literal) : value(value), freq(freq), isMatch(literal), left(nullptr), right(nullptr) {}
    Node(int freq, Node *left, Node *right) : value(std::min(left->value, right->value)), freq(freq), left(left), right(right) {}
};
#endif