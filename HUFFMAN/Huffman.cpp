    #include "Huffman.h"
    #include "Bitstream.h"
    #include <string>
    #include <unordered_map>
    #include <queue>

    std::unordered_map<int, int> caculateFreqs(const std::vector<Token> &tokens) // tính tần suất các giá trị Token
    {
        std::unordered_map<int, int> freqs;
        for (const auto &t : tokens)
            if (t.isMatch == false)
                freqs[(int)(unsigned char)t.literal]++; // đếm tần suất kí tự
            else
            {
                freqs[t.length + lengthSize]++;  // đếm tần suất length
                freqs[t.distance + matchSize]++; // dến tần suất distance
            }
        return freqs;
    }

    std::priority_queue<Node *, std::vector<Node *>, cmp> generateQueue(std::unordered_map<int, int> &freqs) // tạo priority queue
    {
        std::priority_queue<Node *, std::vector<Node *>, cmp> q;
        for (auto const &[v, f] : freqs) // v là key, f là value của map
        {
            bool isMatch = (v > lengthSize);
            q.push(new Node(v, f, isMatch)); // đẩy Node vào queue
        }
        return q;
    }

    Node *generateTree(std::priority_queue<Node *, std::vector<Node *>, cmp> &p_queue) // tạo cây Huffman
    {
        if (p_queue.empty() == true) // queue rỗng
            return nullptr;
        if (p_queue.size() == 1) // queue chỉ có một phần tử
            return p_queue.top();

        while (p_queue.size() > 1)
        {
            Node *left = p_queue.top();
            p_queue.pop();
            Node *right = p_queue.top();
            p_queue.pop();

            Node *parent = new Node(left->freq + right->freq, left, right); // tạo Node tần suất tổng
            // parent->value = std::min(left->value, right->value);
            p_queue.push(parent);
        }
        return p_queue.top();
    }

    void generateCode(Node *root, unsigned int curCode, unsigned char curLen, std::unordered_map<int, BitCode> &mapCodes) // tạo bảng mã
    {
        if (root == nullptr) // cây rỗng
            return;
        if (curLen == 0 && root->left == nullptr && root->right == nullptr) // cây một Node
        {
            mapCodes[root->value] = {0, 1};
            return;
        }

        if (root->left == nullptr && root->right == nullptr)
        {
            mapCodes[root->value] = {curCode, curLen};
            return;
        }
        generateCode(root->left, (curCode << 1), curLen + 1, mapCodes);      // sang Node trái
        generateCode(root->right, (curCode << 1) | 1, curLen + 1, mapCodes); // sang Node phải
    }

    void deleteTree(Node *root) // xóa cây
    {
        if (root == nullptr)
            return;
        deleteTree(root->left);
        deleteTree(root->right);
        delete root;
    }
    std::vector<Token> decodeTokens(InBitStream &inStream, Node *root, size_t totalTokens)
    {
        std::vector<Token> tokens;
        if (root == nullptr)
            return tokens;

        for (size_t i = 0; i < totalTokens; ++i)
        {
            int val = getSymbol(inStream, root);
            if (val == -1)
                break;

            Token t;
            if (val < lengthSize) // literal < 256
            {
                t.isMatch = false;
                t.literal = (char)val;
                t.length = 0;
                t.distance = 0;
            }
            else
            {
                t.isMatch = true;
                t.length = val - lengthSize;
                t.literal = 0;

                int distVal = getSymbol(inStream, root);
                if (distVal == -1)
                    break;

                t.distance = distVal - matchSize;
            }
            tokens.push_back(t);
        }
        return tokens;
    }