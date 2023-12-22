# Тестовое задание YADRO

##Реализация сортировки данных с магнитной ленты

Для эмуляции работы с магнитной лентой был реализован класс MagnetTape. В качестве ленты изпользуются бинарные файлы.
Интерфейс работы с магнитной лентой:
- Загрузить ленту - метод Load класса MagnetTape. В качестве параметра принимает путь к файлу, эмулирующего ленту.
- Выгрузить ленту - метод Unload классф MagnetTape.
- Перемотать ленту на 1 позицию вперед - метод MoveForward класса MagnetTape.
- Перемотать ленту на 1 позицию назад - метод MoveBack класса MagnetTape.
- Считать значение в ячейке под магнитной головкой - метод getCurr класса MagnetTape.
- Записать значение в ячейку под магнитной головкой - метод setCurr класса MagnetTape.

По усливию имеется только M доступной оперативной пямяти. При этом возможно использовать временные ленты.
Таким образом используется сортировка с многопутевым слиянием.
Алгоритм работы сортировки:
 - Из входной ленты последовательно считывается M элементов, после чего считанные данные сортируются во внутренней пямяти и записываются в отсортированном виде на вспомогательную ленту. После чего продолжают считываться данные с исходной магнитной ленты. Это повторяется до тех пор, пока не будут считаны, отсортированные и сохранены на временных лентах все элементы со входной ленты
 - Далее данные из всех временных лент сливаются в результирующую ленту методом многопутевого слияния.
