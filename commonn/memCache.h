
enum {ALIGN = 8};
enum {MAX_BYTES = 128};
enum {LISTS = MAX_BYTES/ALIGN};

class Allocate
{
    private:
    static size_t ROUND_UP(size_t n)
    {
        return (n + ALIGN -1)/(ALIGN);
    } 
    struct obj
    {
        struct obj * free_list_link;
        char client_data[1];
    };
    static obj * free_list[LISTS];
};
