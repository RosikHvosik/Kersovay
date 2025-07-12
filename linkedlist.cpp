#include "linkedlist.h"
#include "array.h"
#include <QDebug>

linkedList::linkedList() : head(nullptr), last(nullptr), size(0) {}

linkedList::~linkedList()
{
    if (!head)
        return;
    lNode *curr = head->next;
    while (curr != head)
    {
        lNode *temp = curr;
        curr = curr->next;
        delete temp;
    }
    delete head;
    head = last = nullptr;
    size = 0;
}

bool linkedList::isEmpty() const
{
    return head == nullptr;
}

void linkedList::add(std::size_t arrayIndex)
{
    lNode *temp = new lNode(arrayIndex);

    if (!head)
    {
        head = last = temp;
        temp->next = head;
    }
    else
    {
        temp->next = head;
        head = temp;
        last->next = head;
    }
    ++size;

    qDebug().noquote() << QString("список: добавлен индекс %1, размер=%2")
                              .arg(arrayIndex)
                              .arg(size);
}

std::string linkedList::show()
{
    if (!head)
        return "Список пуст.";

    std::string result = "[";
    lNode *temp = head;
    do
    {
        //инфа о счетах фулл 2 справочник значит у меня приемы
        const Appointment &appointment = AppointmentArray[temp->arrayIndex];
        result += appointment.diagnosis;
        temp = temp->next;
        if (temp != head)
            result += ", ";
    } while (temp != head);
    result += "]";
    return result;
}
//ищу по приему
int linkedList::searchByAppointment(const std::string& doctor, const std::string& diagnosis, const Date& date)
{
    if (!head)
        return -1;

    lNode *curr = head;
    int index = 0;
    do
    {
        const Appointment &app = AppointmentArray[curr->arrayIndex];
        //проверка всех подей на совпадение т.к нет уникального
        if (app.doctorType == doctor && app.diagnosis == diagnosis && app.appointmentDate == date)
        {
            //тут мб поправить
            qDebug().noquote() << QString("список: найден приём врача %1 с диагнозом %2 на дату %3.%4.%5 на позиции %6")
                                      .arg(QString::fromStdString(doctor))
                                      .arg(QString::fromStdString(diagnosis))
                                      .arg(date.day, 2, 10, QLatin1Char('0'))
                                      .arg(static_cast<int>(date.month), 2, 10, QLatin1Char('0'))
                                      .arg(date.year)
                                      .arg(index);

            return index;
        }
        curr = curr->next;
        ++index;
    } while (curr != head);
    //тут тоже скорее всего
    qDebug().noquote() << QString("список: приём врача %1 с диагнозом %2 на дату %3.%4.%5 не найден")
                              .arg(QString::fromStdString(doctor))
                              .arg(QString::fromStdString(diagnosis))
                              .arg(date.day, 2, 10, QLatin1Char('0'))
                              .arg(static_cast<int>(date.month), 2, 10, QLatin1Char('0'))
                              .arg(date.year);

    return -1;
}

void linkedList::removeAt(int index)
{
    if (!head || index < 0 || index >= size)
        return;

    lNode *curr = head;
    lNode *prev = last;

    for (int i = 0; i < index; ++i)
    {
        prev = curr;
        curr = curr->next;
    }

    qDebug().noquote() << QString("список: удаляем индекс %1 с позиции %2")
                              .arg(curr->arrayIndex)
                              .arg(index);

    if (curr == head)
        head = head->next;
    if (curr == last)
        last = prev;
    prev->next = curr->next;

    delete curr;
    --size;

    if (size == 0)
    {
        head = last = nullptr;
    }
    else
    {
        last->next = head;
    }

    qDebug().noquote() << QString("список: после удаления размер=%1").arg(size);
}

int linkedList::getSize() const
{
    return size;
}

template <typename TreeType>
void linkedList::updateIndices(std::size_t oldIdx, std::size_t newIdx, TreeType &tree)
{
    if (!head)
        return;

    lNode *curr = head;
    do
    {
        if (curr->arrayIndex == oldIdx)
        {
            curr->arrayIndex = newIdx;
            qDebug().noquote() << QString("список: обновлен индекс %1 → %2")
                                      .arg(oldIdx)
                                      .arg(newIdx);
            return;
        }
        curr = curr->next;
    } while (curr != head);
}
