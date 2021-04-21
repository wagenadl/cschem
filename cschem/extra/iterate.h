#ifndef ITERATE_H
#define ITERATE_H

template <typename T>
class asKeyValueRange
{
public:
    asKeyValueRange(T &data)
        : m_data{data}
    {
    }

    auto begin() { return m_data.keyValueBegin(); }

    auto end() { return m_data.keyValueEnd(); }

private:
    T &m_data;
};

#endif // ITERATE_H
