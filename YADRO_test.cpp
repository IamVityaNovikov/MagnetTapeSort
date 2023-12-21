
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <thread>

unsigned read_delay = 0;
unsigned write_delay = 0;
unsigned move_delay = 0;
unsigned M = UINT_MAX;

//класс магнитной ленты
class MagnetTape {
public:
    MagnetTape() {
        curr = 0;
        currPos = -1;
        isLoaded = false;
        N = 0;

    }

    //Создание ленты длинной len c нулевыми значениями
    MagnetTape(std::string filename, int len) {
        tapeFile.open(filename, std::ios::ate | std::ios::out | std::ios::binary | std::ios::trunc);
        if (tapeFile.is_open()) {
            tapeFile.seekp(0);
            int initVal = 0;
            for (int i = 0; i < len; i++) {
                tapeFile.seekp(i * sizeof(int));
                tapeFile.write(reinterpret_cast<char*>(&initVal), sizeof(int));
            }
            tapeFile.seekg(0);
            tapeFile.seekp(0);
            curr = initVal;
            currPos = 0;
            N = len;
            isLoaded = true;
            this->filename = filename;
        }
        else {
            curr = 0;
            currPos = -1;
            isLoaded = false;
            N = 0;
        }

    }

    //открывает магнитную ленту из файла
    MagnetTape(std::string filename) {
        curr = 0;
        currPos = -1;
        isLoaded = false;
        N = 0;
        Load(filename);

    }

    //переместить ленту на 1 вперед
    void MoveForward() {
        if (isLoaded&&tapeFile.is_open()) {
            if (currPos < N - 1) {
                currPos++;
                int newFilePos = currPos * sizeof(int);
                tapeFile.seekp(newFilePos);
                tapeFile.seekg(newFilePos);
                tapeFile.read((char*)&curr, sizeof(int));
                tapeFile.seekp(newFilePos);
                tapeFile.seekg(newFilePos);
                std::this_thread::sleep_for(std::chrono::milliseconds(move_delay));
            }
        }
    }
    //переместить ленту на 1 назад
    void MoveBack() {
        if (isLoaded && tapeFile.is_open()) {
            if (currPos > 0) {
                currPos--;
                int newFilePos = currPos * sizeof(int);
                tapeFile.seekp(newFilePos);
                tapeFile.seekg(newFilePos);
                tapeFile.read((char*)&curr, sizeof(int));
                tapeFile.seekp(newFilePos);
                tapeFile.seekg(newFilePos);
                std::this_thread::sleep_for(std::chrono::milliseconds(move_delay));
            }
        }
    }

    //"загрузка ленты в аппарат" (связывает файл ленты с объектом ленты)
    bool Load(std::string filename) {
        tapeFile.open(filename, std::ios::ate | std::ios::out | std::ios::in | std::ios::binary);
        if (tapeFile.is_open()) {
            tapeFile.seekg(0, std::ios::end);
            N = tapeFile.tellg() / sizeof(int);
            tapeFile.seekg(0);
            tapeFile.seekp(0);
            currPos = 0;
            tapeFile.read((char*)&curr, sizeof(int));
            tapeFile.seekg(0);
            isLoaded = true;
            this->filename = filename;
            return true;
        }
        else {
            tapeFile.open(filename,std::ios::out| std::ios::trunc);
            tapeFile.close();
            tapeFile.open(filename, std::ios::ate | std::ios::out | std::ios::in | std::ios::binary);
            if (tapeFile.is_open()) {
                tapeFile.seekg(0);
                tapeFile.seekp(0);
                currPos = 0;
                curr = 0; 
                isLoaded = true;
                this->filename = filename;
                return true;
            }
            else {
                return false;
            }
        }
    }
    //"Выгрузка ленты из аппарата"(закрытие файла)
    void Unload() {
        if (isLoaded && tapeFile.is_open()) {
            tapeFile.close();
            isLoaded = false;
        }
    }

    //Записать значение в ячейку под магнитной головкой
    void SetCurr(int value) {
        if (isLoaded && tapeFile.is_open()) {
            int filePos = currPos * sizeof(int);
            tapeFile.seekp(filePos);
            tapeFile.seekg(filePos);
            tapeFile.write(reinterpret_cast<char*>(&value), sizeof(int));
            curr = value;
            tapeFile.close();
            tapeFile.open(filename, std::ios::ate | std::ios::out | std::ios::in | std::ios::binary);
            tapeFile.seekp(filePos);
            tapeFile.seekg(filePos);
            tapeFile.flush();
            if (currPos >= N)
                N = currPos + 1;
            std::this_thread::sleep_for(std::chrono::milliseconds(write_delay));

        }
    }

    //Получить позицию головки
    int getCurrPos() {
        return currPos;
    }

    //Получить значение под магнитной головкой
    int getCurr() {
        std::this_thread::sleep_for(std::chrono::milliseconds(read_delay));
        return curr;
    }

    //Узнать длину ленты
    int getLen() {
        return N;
    }

    //Узнать имя связанного с лентой файла
    std::string getFileName() {
        return filename;
    }

    //Узнать загруженна ли лента в аппарат
    bool isTapeLoaded() {
        return isLoaded;
    }


private:
    int N;
    std::string filename;
    bool isLoaded;
    int curr;
    int currPos;
    std::fstream tapeFile;


};


class MagnetTapeManager {

public:

    //сортировка
    //загружается M элементов в память, 
    //элементы сортируются в памяти и записываются на временную ленту
    //после чего все отсортированные временные ленты сливаются в конечную
    void tapeSort(MagnetTape& tape, std::string sortedFileName) {
        //перемотка ленты к началу
        while (tape.getCurrPos() > 0) {
            tape.MoveBack();
        }

        std::vector<int> tempData;
        tempData.reserve(M);
        int count = 0;
        int fileCount = 0;
        std::vector<MagnetTape> inputTapes;

        //данные с ленты записываются во внутреннюю память размером M
        while (count < tape.getLen()) {
            for (; tempData.size() < M; tape.MoveForward()) {
                tempData.push_back(tape.getCurr());
                count++;
                if (count == tape.getLen()) break;
            }

            //Данные сортируются во внутренней памяти
            sort(tempData.begin(), tempData.end());

            std::string tmpFileName = "tmp/" + tape.getFileName() + std::to_string(fileCount);
            //Отсортированные данные записываются на временную ленту
            vectorToTapefile(tempData, tmpFileName);
            inputTapes.push_back(MagnetTape(tmpFileName));
            tempData.clear();
            fileCount++;
        }

        //Лента для записи отсортированных данных
        MagnetTape resTape(sortedFileName, tape.getLen());

        //ленты, доступные для слияния
        std::vector<bool> tapeAvailable(fileCount, true);

        count = 0;

        //слияние
        while (count < tape.getLen()) {
            int minIndex = -1;
            //проход по всем доступным временным лентам
            //поиск ленты с минимальным элементом под магнитной головкой
            for (int i = 0; i < fileCount; i++) {
                if (tapeAvailable[i]) {
                    if (minIndex == -1) {
                        minIndex = i;
                    }
                    else {
                        if (inputTapes[i].getCurr() < inputTapes[minIndex].getCurr()) {
                            minIndex = i;
                        }
                    }
                }
            }
            if (minIndex != -1) {
                //минимальный элемент записывается на ленту отсортированных данных
                //если магнитная головка находилась в конце временной ленты то данная лента уже не учавствует в слиянии
                //иначе временная лента перематывается вперед
                resTape.SetCurr(inputTapes[minIndex].getCurr());
                //std::cout << inputTapes[minIndex].getCurr() << " ";
                resTape.MoveForward();
                if (inputTapes[minIndex].getCurrPos() == inputTapes[minIndex].getLen() - 1) {
                    tapeAvailable[minIndex] = false;
                }
                else {
                    inputTapes[minIndex].MoveForward();
                }
                count++;
            }
        }

        //"выгрузка" всех временных лент
        for (int i = 0; i < inputTapes.size(); i++) {
            inputTapes[i].Unload();
        }
        resTape.Unload();
    }

private:

    //запись из вектора на ленту
    //нужно для создания временных файлов
    void vectorToTapefile(std::vector<int> v, std::string filename) {
        MagnetTape tape(filename, v.size());
        for (int value : v) {
            tape.SetCurr(value);
            tape.MoveForward();
        }
        tape.Unload();
    }
};

int main()
{
    MagnetTapeManager tapeManager;

    std::ifstream paramsFile;
    paramsFile.open("params.txt", std::ios::in);
    if (paramsFile.is_open()) {
        paramsFile >> read_delay;
        paramsFile >> write_delay;
        paramsFile >> move_delay;
        paramsFile >> M;
    }
    paramsFile.close();
    
    tapeManager = MagnetTapeManager();

    std::string inputFileName;
    std::cin >> inputFileName;

    std::string sortedFileName;
    std::cin >> sortedFileName;

    MagnetTape inputTape(inputFileName);
    tapeManager.tapeSort(inputTape, sortedFileName);
    inputTape.Unload();

}
