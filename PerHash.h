#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace PerHash {
    class StringPerHash; // fwd dec

    class AllocationTable {
    public:

        std::vector<int> key_pool;
        std::unordered_map<size_t, uint64_t> registry_reverse;
        std::unordered_map<uint64_t, size_t> registry;
        std::unordered_map<uint64_t, size_t> registry_refs;
        double preallocate = 0;
        static AllocationTable& getInstance();
        void rehash_key(StringPerHash& key);
        void dispose(StringPerHash& key);
    private:
        AllocationTable() = default;

    public:
        AllocationTable(AllocationTable const&) = delete;
        void operator=(AllocationTable const&) = delete;
    };


    class StringHash {



        [[nodiscard]] static size_t generate_hash(const std::string& value_)
        {
            return (value_.empty()) ? 0 : std::hash<std::string>{}(value_);
        }

    protected:
        size_t hash = 0;
    public:


        StringHash() = default;

        virtual ~StringHash() = default;

        explicit StringHash(const std::string& in){
            StringHash::update(in);
        };

        virtual void update(const std::string& string) {
            const auto new_hash = generate_hash(string);
            getAllocationTable().emplace(new_hash, string);
            hash = new_hash;
        }

        inline bool operator==(const StringHash& other) const
        {
            return hash == other.hash;
        }

        inline bool operator<(const StringHash& other) const
        {
            return hash < other.hash;
        }

        [[nodiscard]] std::string get_string() const
        {
            if(getAllocationTable().count(hash) > 0){
                return getAllocationTable().at(hash);
            }
            return "N/A"; // When this happens, is it a bug?

        }

        [[nodiscard]] size_t get_hash() const
        {
            return hash;
        }

        static StringHash make(const std::string& str)
        {
            return StringHash(str);
        }


        using StringAllocationTable = std::unordered_map<uint64_t, std::string>;
        static StringAllocationTable& getAllocationTable() {
            static StringAllocationTable table;
            return table;
        }
    };


    class StringPerHash: public StringHash {

    public:
        StringPerHash() = default;

        explicit StringPerHash(const std::string& in)
        : StringHash(in)
        {
            AllocationTable::getInstance().rehash_key(*this);
        };

        explicit StringPerHash(size_t hash_, int perhash)
        : perhash(perhash)
        {
            hash = hash_;
        };

        void update(const std::string& string) override {
            StringHash::update(string);
            AllocationTable::getInstance().rehash_key(*this);
        }


        void set_perhash(int perhash_)
        {
            perhash = perhash_;
        }

        [[nodiscard]] int get_perhash() const{
            return perhash;
        }

        void dispose(){
            /// WARNING: When disposing a StringPerHash, you do this for ALL of them. (aka all keys will be invalidated)
            AllocationTable::getInstance().dispose(*this);
        }

        int perhash = -1;
    };




    void AllocationTable::rehash_key(StringPerHash& key)
    {
        auto hash = key.get_hash();
        if (registry.count(hash) > 0) {
            registry_refs[hash] += 1;
            auto registry_value = registry.at(hash);
            key.set_perhash(registry_value);

        } else {
            if(!key_pool.empty()){
                auto v = key_pool.back();
                key_pool.pop_back();
                registry[hash] = v;
                registry_reverse[v] = hash;
                registry_refs[hash] = 1;
                key.set_perhash(v);

            }else{
                auto regSize = registry.size();
                registry[hash] = regSize;
                registry_reverse[regSize] = hash;
                registry_refs[hash] = 1;
                key.set_perhash(registry[hash]);
                preallocate += 1;
            }
        }
    }


    void AllocationTable::dispose(StringPerHash &key) {
        const auto hash = key.get_hash();
        const auto refs_c = registry_refs[hash];

        if(refs_c == 1){
            //SPDLOG_INFO("DEREF {} Refcount: {}", key.get_string(), refs_c);
            auto perhash = key.get_perhash();
            registry_refs.erase(hash);
            key_pool.push_back(perhash);
            registry.erase(hash);
            registry_reverse.erase(perhash);
        }else{
            registry_refs[hash] -= 1;
        }
    }


    AllocationTable& AllocationTable::getInstance()
    {
        static AllocationTable instance;
        return instance;
    }






    template <typename K, typename V>
    class HashMap {
        using data_type = V;
        using iterator = typename std::vector<data_type>::const_iterator;

        std::vector<data_type> data;
        static constexpr double DEFAULT_EXPAND_RATIO = 0.2;
        const double expand_ratio;


    public:

        explicit HashMap(size_t allocate, double expand_ratio = DEFAULT_EXPAND_RATIO)
        : data(std::ceil(AllocationTable::getInstance().preallocate + allocate))
        , expand_ratio(expand_ratio)
        {
        }

        explicit HashMap()
        : data(std::ceil(AllocationTable::getInstance().preallocate))
        , expand_ratio(DEFAULT_EXPAND_RATIO)
        {

        }

        const V& at(const K& key) const
        {
            return data[key.perhash];
        }


        V& at(const K& key)
        {
            return data[key.perhash];
        }

        V& at(int i){
            return data[i];
        }

        template<class D, class E>
        void emplace(D &key, E value)
        {
            check_do_allocation(key);
            auto &container = data[key.perhash];
            container = value;
        }

        void emplace(K&& key, V&& value)
        {
            check_do_allocation(key);
            auto &container = data[key.perhash];
            container = std::move(value);
        }

        void clear()
        {
            data.clear();
        }

        void resize(size_t size)
        {
            data.resize(size);
        }

        [[nodiscard]] size_t size() const
        {
            return data.size();
        }

        iterator begin() const
        {
            return data.begin();
        }

        iterator end() const
        {
            return data.end();
        }

        void calculate_allocation(const K& key, size_t data_size)
        {
            auto next = std::ceil( (data_size * expand_ratio) + data_size);
            PerHash::AllocationTable::getInstance().preallocate = next;
        }

        void check_do_allocation(const K& key)
        {
            int data_size = static_cast<int>(data.size());
            if (data_size > key.perhash) {
                return;
            }

            calculate_allocation(key, data_size);

            auto newSize = std::max(
                    std::lround(AllocationTable::getInstance().preallocate + 1),
                    static_cast<int64_t>(key.perhash) + 1);
            resize(newSize);

        }


        [[nodiscard]] int capacity()const{
            return data.capacity();
        }

        V at_index(int idx) const {
            return data.at(idx);
        }
    };

} // namespace PerHash