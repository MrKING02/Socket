#include "server.h"


int main()
{
    thrd_t thread;
    thrd_create(&thread, (void *)start, NULL);

    thrd_join(thread, NULL);

    return 0;
}