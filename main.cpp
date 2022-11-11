#include <stdlib.h>
#include <unistd.h>
#include <ctime>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <cstring>
#include <string>
#include <wait.h>
#include "check.hpp"
#include <iostream>
using namespace std;
int pid1, pid2, result = 0, count = 1, module = 10;
bool isParentWish = true;
vector<bool> numbers(21, false);
void make_a_number() {        //загадать число
    result = rand() % 21 + 1;
    cout << "Загаданное число: " << result << "\n";
}
int get_value() {
    int index = rand() % 21 + 1;
    while (numbers[index]) {
        index = rand() % 21 + 1;
    }
    numbers[index] = true;
    return index;
}
void unravel_a_number(mqd_t mqd) {    //отгадать число
    clock_t t = clock();
    while (true) {
        int tmp = get_value();
        cout << "Попытка " << count++ << ": " << tmp << "\n";
        sleep(1);
        check(mq_send(mqd, (char*)&tmp, sizeof(tmp), 0));
        //sleep(1);
        int* buf = new int[8 * 1024 / sizeof(int)];
        check(mq_receive(mqd, (char*)buf, 8*1024, NULL));
        if (buf[0] == -1) {
            if (pid2 == 0) cout << "Дочерний процесс угадал число!\n";
            else cout << "Родительский процесс угадал число!\n";
            cout << "Количество попыток: " << count - 1 << "\n";
            cout << "Затраченное время: " << double(clock() - t) / CLOCKS_PER_SEC << "\n";
            count = 1;
            for (int i = 0; i < numbers.size(); i++) {
                numbers[i] = false;
            }
//            int c = -3;
//            check(mq_send(mqd, (char*)&c, sizeof(c), 0));
            break;
        }
    }
}
void work(bool flag, mqd_t mqd) {
    if ((isParentWish && flag) || (!isParentWish && !flag)) {
        if (flag) {
            cout << "Загадывает родительский процесс, угадывает дочерний процесс\nРодительский процесс начал работу!\n";
        }
        else {
            cout << "Загадывает дочерний процесс, угадывает родительский\nДочерний процесс начал работу!\n";
        }
        make_a_number();
        //int a = -4;
        //cout << check(mq_send(mqd, (char*)&a, sizeof(a), 0));
        sleep(1);
        while (true) {
            int* buf = new int[8*1024/sizeof(int)];
            check(mq_receive(mqd, (char*)buf, 8*1024, 0));
            cout << buf[0];
            if (buf[0] == result) {
                int b = -1;
                check(mq_send(mqd, (char*)&b, sizeof(b), 0));
                sleep(1);
                break;
            }
            else {
                int b = -2;
                check(mq_send(mqd, (char*)&b, sizeof(b), 0));
                sleep(1);
            }
        }
        //int* buf = new int[8*1024 / sizeof(int)];
        //check(mq_receive(mqd, (char*)buf, 8 * 1024, 0));
        if (flag) isParentWish = false;
        else isParentWish = true;
    }
    else
    {
        if (flag) cout << "Родительский процесс начал работу!\n";
        else cout << "Дочерний процесс начал работу!\n";
        //int* buf = new int[8*1024/sizeof(int)];
        //check(mq_receive(mqd, (char*)buf, 8*1024, NULL));
        //cout << buf[0];
        //if (buf[0] == -4) {
        //    cout << "Сообщение получено!\n";
        //    unravel_a_number(mqd);
        //}
        unravel_a_number(mqd);
        if (flag) isParentWish = true;
        else isParentWish = false;
    }
}
int main() {
    srand(11 + time(NULL));
    pid1 = getpid();
    cout << "Родительский процесс: " << pid1 << "\n";
    {
        auto d = check(mq_open("/mymq", O_CREAT, S_IRUSR | S_IWUSR, NULL));
        check(mq_close(d));
    }
    pid2 = fork();
    for (int i = 0; i <= 2; i++) {
        if (pid2 != 0) { //родительский процесс
            mqd_t mqd = check(mq_open("/mymq", O_RDWR));
            if (mqd == -1) return -1;
            work(true, mqd);

        }
        else { //дочерний процесс
            srand(10 + time(NULL));
            cout << "Дочерний процесс: " << getpid() << "\n";
            mqd_t mqd = check(mq_open("/mymq", O_RDWR));
            if (mqd == -1) return -1;
            work(false, mqd);
        }
    }


}



#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include "check.hpp"

//int child(mqd_t mqd) {
//    int* buf = new int[8 * 1024 / sizeof(int)];
//    int r = 0;
//    int i = 500;
//    do {
//        r = check(mq_receive(mqd, (char*)buf, 8 * 1024, NULL));
//        sleep(1);
//        printf("Child >> Got %d from the queue\n", buf[0]);
//        if (buf[0] == -1) break;
//        int a = 7;
//        printf("Child >> Writing 7 into the queue\n");
//        sleep(1);
//        check(mq_send(mqd, (char*)&a, sizeof(i), 0));
//        r++;
//    } while (r < 10);
//    delete[] buf;
//}
//
//int parent(mqd_t mqd) {
//    int i = 0;
//    int* buf = new int[8 * 1024 / sizeof(int)];
//    int index = 4;
//    while (true) {
//        printf("Parent >> Writing %d into the queue\n", index);
//        check(mq_send(mqd, (char *) &index, sizeof(i), 0));
//        index++;
//        sleep(1);
//        check(mq_receive(mqd, (char *) buf, 8 * 1024, NULL));
//        printf("Parent >> Got %d from the queue\n", buf[0]);
//        sleep(1);
//        if (index >= 10) {
//            i = -1;
//            check(mq_send(mqd, (char *) &i, sizeof(i), 0));
//            break;
//        }
//
//    }
//    //i = -1;
//    //send end message
//    //check(mq_send(mqd, (char*)&i, sizeof(i), 0));
//}
//
//int main() {
//    { //ensure the message queue exists
//        auto d = check(mq_open("/mymq", O_CREAT, S_IRUSR | S_IWUSR, NULL));
//        check(mq_close(d)); //free descriptor
//    }
//    pid_t p = check(fork());
//    if (p) {
//        mqd_t mqd = check(mq_open("/mymq",O_RDWR)); //open for write
//        parent(mqd);
//        int stat;
//        wait(&stat);
//        mq_unlink("/mymq"); //remove the queue from the filesystem
//    }
//    else {
//        mqd_t mqd = check(mq_open("/mymq", O_RDWR)); //open for read
//        child(mqd);
//    }
//}

