#include <Windows.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include<direct.h>

class Tape {

    public:

        Tape(const char* filename, bool isExist = false) {
            if (isExist) {
                if ((file = fopen(filename, "rb+")) == NULL) {
                    std::cout << "Cannot open file." << std::endl;
                }
            } else {
                if ((file = fopen(filename, "wb+")) == NULL) {
                    std::cout << "Cannot open file." << std::endl;
                }
            }
        }

        virtual ~Tape() {
            fclose(file);
        }

        int moveLeft() {
            Sleep(moveDelay);
            return stepLeft();
        }

        int moveRight() {
            Sleep(moveDelay);
            return stepRight();
        }

        int read(int& num) {
            Sleep(readDelay);
            if (fread(&num, sizeof(int), 1, file)) {
                stepLeft();
                return 0;
            }
            return 1;
        }

        int write(int num) {
            Sleep(writeDelay);
            if (fwrite(&num, sizeof(int), 1, file)) {
                stepLeft();
                return 0;
            }
            return 1;
        }

        void rewind() {
            while (moveLeft() == 0) {}
        }

        static void setReadDelay(int delay) {
            readDelay = delay;
        }

        static void setWriteDelay(int delay) {
            writeDelay = delay;
        }

        static void setMoveDelay(int delay) {
            moveDelay = delay;
        }


    private:

        FILE* file;
        static int readDelay;
        static int writeDelay;
        static int moveDelay;

        int stepLeft() {
            return fseek(file, -sizeof(int), SEEK_CUR);
        }

        int stepRight() {
            return fseek(file, sizeof(int), SEEK_CUR);
        }
};

int Tape::readDelay;
int Tape::writeDelay;
int Tape::moveDelay;


class TapeSorter {

    public:

        static int sort(Tape& inputTape, Tape& outputTape) {
            mkdir(".\\tmp\\");
            Tape buffer1(".\\tmp\\tmpTape1.bin");
            Tape buffer2(".\\tmp\\tmpTape2.bin");
            Tape buffer3(".\\tmp\\tmpTape3.bin");

            int num;
            len = 0;

            while(inputTape.read(num) == 0) {
                len++;
                inputTape.moveRight();
                buffer3.write(num);
                buffer3.moveRight();
            }
            buffer3.rewind();

            if (len == 0) {
                std::cout << "Tape is empty";
                return 1;
            } else if (len == 1) {
                buffer3.read(num);
                outputTape.write(num);
                return 0;
            }

            inputLenL = inputLenR = 0;
            outputLenL = outputLenR = 0;
            while (buffer3.read(num) == 0) {
                buffer3.moveRight();
                
                buffer1.write(num);
                buffer1.moveRight();
                inputLenL++;
                if (buffer3.read(num)) {
                    break;
                }
                buffer3.moveRight();
                
                buffer2.write(num);
                buffer2.moveRight();
                inputLenR++;
            }

            buffer3.rewind();
            buffer1.rewind();
            buffer2.rewind();

            outputTapeL = &outputTape;
            outputTapeR = &buffer3;

            inputTapeL = &buffer1;
            inputTapeR = &buffer2;

            curOutputTape = outputTapeR;
            curOutputLen = &outputLenR;

            int numL, numR;
            
            for (int gap = 1; gap < len; gap = gap*2) {

                inputTapeL->read(numL);
                inputTapeR->read(numR);

                tapePositionL = tapePositionR = 0;

                while (tapePositionL < inputLenL || tapePositionR < inputLenR) {

                    curOutputLen = (curOutputTape == outputTapeL) ? &outputLenR : &outputLenL;
                    curOutputTape = (curOutputTape == outputTapeL) ? outputTapeR : outputTapeL;

                    gapPositionL = gapPositionR = 0;

                    if (tapePositionL < inputLenL && tapePositionR >= inputLenR) {
                        while (tapePositionL < inputLenL) {
                            copy(*inputTapeL, tapePositionL, numL);
                        }
                        break;
                    } else if (tapePositionR < inputLenR && tapePositionL >= inputLenL) {
                        while (tapePositionR < inputLenR) {
                            copy(*inputTapeR, tapePositionR, numR);
                        }
                        break;
                    }

                    for (int i = 0; i < gap*2;) {

                        if (numL < numR) {
                            merge(i, gap, *inputTapeL, *inputTapeR,
                                    tapePositionL, tapePositionR,
                                    gapPositionL, gapPositionR, inputLenL, inputLenR,
                                    numL, numR);
                        } else {
                            merge(i, gap, *inputTapeR, *inputTapeL,
                                    tapePositionR, tapePositionL,
                                    gapPositionR, gapPositionL, inputLenR, inputLenL,
                                    numR, numL);
                        }
                        
                        if (tapePositionL >= inputLenL && tapePositionR >= inputLenR) {
                            break;
                        }
                    }
                }

                inputTapeL->rewind();
                inputTapeR->rewind();
                outputTapeL->rewind();
                outputTapeR->rewind();

                std::swap(inputTapeL, outputTapeL);
                std::swap(inputTapeR, outputTapeR);
                std::swap(inputLenL, outputLenL);
                std::swap(inputLenR, outputLenR);

                outputLenL = outputLenR = 0;
            }

            if (&outputTape != inputTapeL) {
                while (inputTapeL->read(num) == 0) {
                    inputTapeL->moveRight();

                    outputTape.write(num);
                    outputTape.moveRight();
                }
            }

            outputTape.rewind();

            return 0;
        }

    private:

        static int len;
        static int inputLenL, inputLenR;
        static int outputLenL, outputLenR;
        static int tapePositionL, tapePositionR;
        static int gapPositionL, gapPositionR;
        static Tape* outputTapeL;
        static Tape* outputTapeR;
        static Tape* inputTapeL;
        static Tape* inputTapeR;
        static Tape* curOutputTape;
        static int* curOutputLen;


        static void copy(Tape& inputTape, int& tapePosition, int& num) {
            curOutputTape->write(num);
            curOutputTape->moveRight();
            inputTape.moveRight();
            inputTape.read(num);

            (*curOutputLen)++;
            tapePosition++;
        }


        static void merge(int& i, int& gap,
                Tape& inputTape, Tape& inputTapeAlt,
                int& tapePosition, int& tapePositionAlt,
                int& gapPosition, int& gapPositionAlt,
                int& inputLen, int& inputLenAlt,
                int& num, int& numAlt) {

            copy(inputTape, tapePosition, num);

            gapPosition++;
            i++;

            if (tapePosition >= inputLen || gapPosition >= gap) {
                    
                while (tapePositionAlt < inputLenAlt && gapPositionAlt < gap) {

                    copy(inputTapeAlt, tapePositionAlt, numAlt);

                    gapPositionAlt++;
                    i++;
                }
            }
        }

};

int TapeSorter::len;
int TapeSorter::inputLenL;
int TapeSorter::inputLenR;
int TapeSorter::outputLenL;
int TapeSorter::outputLenR;
int TapeSorter::tapePositionL;
int TapeSorter::tapePositionR;
int TapeSorter::gapPositionL;
int TapeSorter::gapPositionR;
Tape* TapeSorter::outputTapeL;
Tape* TapeSorter::outputTapeR;
Tape* TapeSorter::inputTapeL;
Tape* TapeSorter::inputTapeR;
Tape* TapeSorter::curOutputTape;
int* TapeSorter::curOutputLen;

int main(int argc, char **argv) {

    std::ifstream configFile("config.txt");
    if (!configFile.is_open()) {
        std::cout << "Cannot open config file." << std::endl;
        return 1;
    }

    std::string line;
    while (getline(configFile, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
        
        if (line[0] == '#' || line.empty()) {
            continue;
        }

        auto delimiterPos = line.find("=");
        auto name = line.substr(0, delimiterPos);
        auto value = line.substr(delimiterPos + 1);

        if (name == "read_delay") {
            Tape::setReadDelay(stoi(value));
        } else if (name == "write_delay") {
            Tape::setWriteDelay(stoi(value));
        } else if (name == "move_delay") {
            Tape::setMoveDelay(stoi(value));
        }
    }
    
    Tape inputTape(argv[1], true);
    Tape outputTape(argv[2]);

    std::cout << "Sorting, please stand by... " << std::endl;
    if (TapeSorter::sort(inputTape, outputTape)) {
        return 1;
    }

    char ans;
    std::cout << "Print sorted tape to cosole? Y/n ";
    std::cin >> ans;

    if (tolower(ans) == 'y') {
        int num;
        while (outputTape.read(num) == 0) {
            std::cout << num << " ";
            outputTape.moveRight();
        }
    }
}

/*
Допущения при реализации:
    Ленты работают по принципу кассет, т.е. при переключении между несколькими кассетами
        они сохраняют своё положение относительно магнитной головки.
    Для эмуляции лент используются бинарные файлы, в которых записаны только integer элементы
        без разделительных знаков.
    Для сортировки применяется алгоритм bottom-up merge sort.
    Используются три временные ленты, как минимальное количество, необходимое для сохранения
        информации на входной ленте и по минимуму использующее перемотку в начало лент.
    Единовременно в оперативной памяти хранится 2 значения элементов с ленты и до 12 целочисленных
        значений позиции магнитной головки и количества данных на лентах.
    Имя входного и выходного файлов передаются при запуске приложения в виде аргументов командной строки.
*/