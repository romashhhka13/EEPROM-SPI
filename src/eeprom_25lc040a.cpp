#include "eeprom_25lc040a.hpp"
#include <algorithm>

/**
 * @brief Прочитать один байт из EEPROM.
 */
uint8_t EEPROM25LC040A::readByte(std::size_t address) const
{
    // Опускаем CS
    spi_.driver().cs_low();

    // Команда READ. EEPROM понимает, что дальше будет адрес и чтение данных
    spi_.transferByte(static_cast<uint8_t>(Opcode::READ));

    // Передаём адрес (для 25LC040A используется 9-битный адрес, т. к. 512 байт,
    // но передаётся как 16 бит: старший байт первым)
    spi_.transferByte(static_cast<uint8_t>((address >> 8) & 0xFF));
    spi_.transferByte(static_cast<uint8_t>(address & 0xFF));

    // Чтение данных (0xFF - фиктивный байт, так как полный дуплекс)
    uint8_t value = spi_.transferByte(0xFF);

    // Поднимаем CS
    spi_.driver().cs_high();

    return value;
}

/**
 * @brief Записать один байт в EEPROM.
 */
void EEPROM25LC040A::writeByte(std::size_t address, uint8_t value)
{
    // Разрешаем запись
    writeEnable();

    // Опускаем CS
    spi_.driver().cs_low();

    // Команда WRITE
    spi_.transferByte(static_cast<uint8_t>(Opcode::WRITE));

    // Передаём адрес
    spi_.transferByte(static_cast<uint8_t>((address >> 8) & 0xFF));
    spi_.transferByte(static_cast<uint8_t>(address & 0xFF));

    // Данные
    spi_.transferByte(value);

    // Поднимаем CS - здесь начинается внутренняя запись
    spi_.driver().cs_high();

    // Ждём окончания записи
    waitUntilWriteComplete();
}

/**
 * @brief Послать команду Write Enable.
 */
void EEPROM25LC040A::writeEnable()
{
    // Опускаем CS
    spi_.driver().cs_low();

    // Команда EEPROM для установки защелки разрешения записи
    spi_.transferByte(static_cast<uint8_t>(Opcode::WREN));

    // Поднимаем CS
    spi_.driver().cs_high();
}

/**
 * @brief Прочитать регистр статуса.
 */
uint8_t EEPROM25LC040A::readStatus() const
{
    // Опускаем CS
    spi_.driver().cs_low();

    // Команда EEPROM для чтения регистра статуса
    spi_.transferByte(static_cast<uint8_t>(Opcode::RDSR));

    // Получение регистра статуса
    uint8_t status = spi_.transferByte(0xFF);

    // Поднимаем CS
    spi_.driver().cs_high();

    return status;
}

/**
 * @brief Ожидать завершения операции записи.
 */
void EEPROM25LC040A::waitUntilWriteComplete() const
{
    // Бит WIP (Write In Progress) — бит 0
    constexpr uint8_t WIP_MASK = 0x01;

    while (true)
    {
        // Проверяем, если запись завершена, status (WIP) = 0x00
        uint8_t status = readStatus();
        if ((status & WIP_MASK) == 0)
        {
            break; // Запись завершена
        }
        // Небольшая пауза
        spi_.driver().delay_us(10);
    }
}

/**
 * @brief Прочитать массив байт из EEPROM.
 */
void EEPROM25LC040A::readArray(std::size_t address,
                               uint8_t *buffer,
                               std::size_t length) const
{
    if (buffer == nullptr || length == 0)
    {
        return;
    }

    // Опускаем CS
    spi_.driver().cs_low();

    // Команда READ
    spi_.transferByte(static_cast<uint8_t>(Opcode::READ));

    // Адрес
    spi_.transferByte(static_cast<uint8_t>((address >> 8) & 0xFF));
    spi_.transferByte(static_cast<uint8_t>(address & 0xFF));

    // Читаем данные
    for (std::size_t i = 0; i < length; ++i)
    {
        buffer[i] = spi_.transferByte(0xFF);
    }

    // Поднимаем CS
    spi_.driver().cs_high();
}

/**
 * @brief Записать массив байт в EEPROM.
 */
void EEPROM25LC040A::writeArray(std::size_t address,
                                const uint8_t *buffer,
                                std::size_t length)
{
    if (buffer == nullptr || length == 0)
    {
        return;
    }

    // remainnig - сколько ещё байт нужно записать
    std::size_t remaining = length;

    // offset - откуда в буфере читать данные
    std::size_t offset = 0;

    while (remaining > 0)
    {
        // Сколько байт можно записать в текущую страницу (размер страницы 16 байт)
        std::size_t page_offset = address % PAGE_SIZE;
        std::size_t bytes_in_page = PAGE_SIZE - page_offset;
        std::size_t chunk = (remaining < bytes_in_page)
                                ? remaining
                                : bytes_in_page;

        // Разрешаем запись
        writeEnable();

        // Опускаем CS
        spi_.driver().cs_low();

        // Команда WRITE
        spi_.transferByte(static_cast<uint8_t>(Opcode::WRITE));

        // Адрес
        spi_.transferByte(static_cast<uint8_t>((address >> 8) & 0xFF));
        spi_.transferByte(static_cast<uint8_t>(address & 0xFF));

        // Записываем данные страницы
        for (std::size_t i = 0; i < chunk; ++i)
        {
            spi_.transferByte(buffer[offset + i]);
        }

        // Поднимаем CS
        spi_.driver().cs_high();

        // Ждём окончания записи страницы
        waitUntilWriteComplete();

        // Переходим дальше
        address += chunk;
        offset += chunk;
        remaining -= chunk;
    }
}

bool EEPROM25LC040A::readBit(std::size_t address, unsigned bit) const
{
    return readBits(address, bit, 1) != 0;
}

void EEPROM25LC040A::writeBit(std::size_t address, unsigned bit, bool value)
{
    writeBits(address, bit, 1, value ? 1u : 0u);
}

uint32_t EEPROM25LC040A::readBits(std::size_t address,
                                  unsigned bitOffset,
                                  unsigned bitCount) const
{
    // Можно прочитать до 32 бит
    if (bitCount == 0 || bitCount > 32)
    {
        return 0;
    }

    uint32_t result = 0;             // Собираем биты
    unsigned bits_read = 0;          // Сколько бит прочитали
    std::size_t byte_addr = address; // Адрес текущего байта EEPROM

    while (bits_read < bitCount)
    {
        // Читаем байт из EEPROM
        uint8_t byte = readByte(byte_addr);

        // Если только начали читать, берём смещение, иначе 0
        unsigned start_bit = (bits_read == 0) ? bitOffset : 0;

        // Сколько бит взять из текущего байта
        unsigned bits_in_byte = std::min(8 - start_bit, bitCount - bits_read);

        // Формирование маски
        uint8_t mask = ((1u << bits_in_byte) - 1) << start_bit;

        // Извлекаем биты
        uint8_t val = (byte & mask) >> start_bit;

        // Кладем в результат
        result |= static_cast<uint32_t>(val) << bits_read;

        // Обновляем счётчики
        bits_read += bits_in_byte;
        ++byte_addr;
    }

    return result;
}

void EEPROM25LC040A::writeBits(std::size_t address,
                               unsigned bitOffset,
                               unsigned bitCount,
                               uint32_t value)
{
    if (bitCount == 0 || bitCount > 32)
    {
        return;
    }

    unsigned bits_written = 0;       // Сколько бит записали
    std::size_t byte_addr = address; // Адрес текущего байта

    while (bits_written < bitCount)
    {
        // Читаем текущий байт
        uint8_t byte = readByte(byte_addr);

        // Если только начали записывать, берём смещение, иначе 0
        unsigned start_bit = (bits_written == 0) ? bitOffset : 0;

        // Сколько бит записать надо в байт
        unsigned bits_in_byte = std::min(8 - start_bit, bitCount - bits_written);

        // Формируем маску
        uint8_t mask = ((1u << bits_in_byte) - 1) << start_bit;

        // Очистка битов и вставка новых
        // (byte & ~mask) - очистка битов (0 - где, перезаписываем)
        byte = (byte & ~mask) | (((value >> bits_written) << start_bit) & mask);

        // Записываем байт
        writeByte(byte_addr, byte);

        // Обновляем счётчики
        bits_written += bits_in_byte;
        ++byte_addr;
    }
}
