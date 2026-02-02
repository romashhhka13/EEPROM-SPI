#ifndef SPI_BITBANGING_HELPER_HPP
#define SPI_BITBANGING_HELPER_HPP

#include <cstdint>
#include "spi_bit_banging_driver.hpp"

/**
 * @file spi_bit_banging_helper.hpp
 * @brief Вспомогательный класс для байтовой передачи по SPI (bit-banging).
 *
 * Класс инкапсулирует типовую логику передачи одного байта по SPI.
 *
 * Используется устройствами более высокого уровня
 * (EEPROM, NOR Flash).
 */

/**
 * @brief Helper-класс для передачи данных по SPI (MSB-first -
 * сначала передаётся старший бит, потом младшие).
 *
 * Данный класс не знает ничего о конкретном SPI-устройстве.
 * Его единственная задача — корректно передать и принять байт
 * через SPIBitBangingDriver.
 */
class SPIBitBangingHelper
{
public:
    /**
     * @brief Конструктор.
     *
     * @param driver Ссылка на существующий bit-bang SPI драйвер.
     */
    explicit SPIBitBangingHelper(SPIBitBangingDriver &driver)
        : driver_(driver) {}

    /**
     * @brief Передать один байт по SPI и одновременно принять ответный байт.
     *
     * Передача выполняется в режиме MSB-first.
     * Для каждого бита:
     *  - выставляется MOSI
     *  - формируется тактовый импульс SCLK
     *  - считывается MISO
     *
     * @param tx_byte Байт для передачи.
     * @return Принятый байт.
     */
    uint8_t transferByte(uint8_t tx_byte)
    {
        uint8_t rx_byte = 0; // Буфер для принятого байта

        for (int bit = 7; bit >= 0; --bit)
        {
            const bool mosi_value = (tx_byte >> bit) & 0x01u; // Берём один бит из передаваемого байта
            driver_.write_mosi(mosi_value);                   // Выставляем его на MOSI

            driver_.pulse_clock(); // Генерируем такт, EEPROM считывает MOSI, выставляет MISO

            const bool miso_value = driver_.read_miso();                             // Считываем MISO
            rx_byte = static_cast<uint8_t>((rx_byte << 1) | (miso_value ? 1u : 0u)); // Сдигаем, добавляем бит
        }

        return rx_byte;
    }

    /**
     * @brief Получить доступ к используемому драйверу.
     *
     * Может быть полезно устройствам более высокого уровня
     * для управления линией CS или задержками.
     *
     * @return Ссылка на SPIBitBangingDriver.
     */
    SPIBitBangingDriver &driver() { return driver_; }

private:
    SPIBitBangingDriver &driver_;
};

#endif // SPI_BITBANG_HELPER_HPP
