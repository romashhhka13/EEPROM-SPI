#ifndef EEPROM_25LC040A_HPP
#define EEPROM_25LC040A_HPP

#include <cstddef>
#include <cstdint>
#include "spi_bit_banging_helper.hpp"

/**
 * @file eeprom_25lc040a.hpp
 * @brief Класс для работы с SPI EEPROM 25LC040A.
 *
 * Реализует высокоуровневый интерфейс доступа к памяти:
 *  - чтение / запись битов
 *  - чтение / запись байтов
 *  - чтение / запись массивов байт
 *
 * Вся работа выполняется через SPIBitBangingHelper,
 * что позволяет полностью абстрагироваться от GPIO и таймингов.
 */

/**
 * @brief Драйвер SPI EEPROM микросхемы 25LC040A.
 *
 * Класс инкапсулирует:
 *  - набор SPI-команд (opcodes)
 *  - формат адреса
 *  - семантику операций записи и чтения
 *
 * Не зависит от конкретной реализации SPI-драйвера.
 */
class EEPROM25LC040A
{
public:
    /**
     * @brief Размер EEPROM в байтах.
     */
    static constexpr std::size_t CAPACITY_BYTES = 512;

    /**
     * @brief Размер страницы записи (page write).
     */
    static constexpr std::size_t PAGE_SIZE = 16;

    /**
     * @brief Конструктор.
     *
     * @param spi_helper Helper для передачи байтов по SPI.
     */
    explicit EEPROM25LC040A(SPIBitBangingHelper &spi_helper)
        : spi_(spi_helper) {}

    /**
     * @brief Прочитать один байт из EEPROM.
     *
     * @param address Адрес в диапазоне [0, CAPACITY_BYTES - 1].
     * @return Прочитанный байт.
     */
    uint8_t readByte(std::size_t address) const;

    /**
     * @brief Записать один байт в EEPROM.
     *
     * Перед записью должна быть выполнена команда Write Enable.
     *
     * @param address Адрес в диапазоне [0, CAPACITY_BYTES - 1].
     * @param value   Байт для записи.
     */
    void writeByte(std::size_t address, uint8_t value);

    /**
     * @brief Прочитать массив байт из EEPROM.
     *
     * @param address Начальный адрес.
     * @param buffer  Буфер назначения (куда сохраняем данные).
     * @param length  Количество байт для чтения.
     */
    void readArray(std::size_t address, uint8_t *buffer, std::size_t length) const;

    /**
     * @brief Записать массив байт в EEPROM.
     *
     * Запись должна учитывать границы страниц памяти.
     *
     * @param address Начальный адрес.
     * @param buffer  Буфер источника (данные).
     * @param length  Количество байт для записи.
     */
    void writeArray(std::size_t address, const uint8_t *buffer, std::size_t length);

    /**
     * @brief Прочитать один байт.
     *
     * @param address Адрес байта.
     * @param bit Номер бита в байте.
     * @return true - логическая 1.
     * @return false 0 - логический 0.
     */
    bool readBit(std::size_t address, unsigned bit) const;

    /**
     * @brief Записать один бит.
     *
     * @param address Адрес байта.
     * @param bit Номер бита в байте
     * @param value Значение, которое нужно записать
     */
    void writeBit(std::size_t address, unsigned bit, bool value);

    /**
     * @brief Прочитать несколько бит из EEPROM.
     *
     * Операция выполняется программно на основе чтения байтов.
     *
     * @param address   Начальный адрес.
     * @param bitOffset Смещение в битах от начального адреса (с какого бита в первом байте начинаем).
     * @param bitCount  Количество бит, которое нужно прочитать.
     * @return Значение, выровненное по младшим битам.
     */
    uint32_t readBits(std::size_t address,
                      unsigned bitOffset,
                      unsigned bitCount) const;

    /**
     * @brief Записать несколько бит в EEPROM.
     *
     * Реализуется через чтение и запись байтов.
     *
     * @param address   Начальный адрес.
     * @param bitOffset Смещение в битах.
     * @param bitCount  Количество бит.
     * @param value     Значение (используются младшие bitCount бит).
     */
    void writeBits(std::size_t address,
                   unsigned bitOffset,
                   unsigned bitCount,
                   uint32_t value);

private:
    /**
     * @brief Команды SPI EEPROM.
     */
    enum class Opcode : uint8_t
    {
        READ = 0x03,  ///< Read data (Чтение данных)
        WRITE = 0x02, ///< Write data (Запись данных)
        WREN = 0x06,  ///< Write enable (Установка защелки разрешения записи)
        RDSR = 0x05   ///< Read status register (Чтение регистра статуса для проверки готовности)
    };

    /**
     * @brief Послать команду Write Enable.
     */
    void writeEnable();

    /**
     * @brief Прочитать регистр статуса.
     *
     * @return Значение регистра статуса.
     */
    uint8_t readStatus() const;

    /**
     * @brief Ожидать завершения операции записи.
     *
     * Метод опрашивает бит WIP (Write In Progress)
     * в регистре статуса.
     */
    void waitUntilWriteComplete() const;

private:
    SPIBitBangingHelper &spi_;
};

#endif // EEPROM_25LC040A_HPP
