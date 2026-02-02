#ifndef SPI_BITBANGING_DRIVER_HPP
#define SPI_BITBANGING_DRIVER_HPP

#include <cstdint>

/**
 * @file spi_bit_banging_driver.hpp
 * @brief Абстрактный интерфейс для SPI bit-banging драйвера.
 *
 * Данный интерфейс инкапсулирует управление линиями SPI
 * (CS, MOSI, MISO, SCLK) и таймингами.
 */

/**
 * @brief Абстрактный интерфейс SPI драйвера с побитовым управлением.
 *
 * Класс определяет минимальный набор операций, необходимых
 * для реализации SPI протокола в режиме bit-banging.
 *
 * Реализации этого интерфейса:
 * - управляют GPIO
 * - обеспечивают корректные временные интервалы
 * - не содержат логики конкретных SPI-устройств
 */
class SPIBitBangingDriver
{
public:
    /**
     * @brief Виртуальный деструктор.
     *
     * Необходим для корректного удаления объектов
     * через указатель на базовый класс.
     */
    virtual ~SPIBitBangingDriver() = default;

    /**
     * @brief Установить линию CS в активное состояние (LOW).
     *
     * Обычно активный уровень CS — низкий.
     * Метод должен быть блокирующим и завершаться
     * только после фактического изменения уровня.
     */
    virtual void cs_low() = 0;

    /**
     * @brief Установить линию CS в неактивное состояние (HIGH).
     */
    virtual void cs_high() = 0;

    /**
     * @brief Установить значение на линии MOSI.
     *
     * @param bit Значение бита для передачи:
     *            - true  — логическая 1
     *            - false — логический 0
     */
    virtual void write_mosi(bool bit) = 0;

    /**
     * @brief Считать текущее значение линии MISO.
     *
     * @return true  — логическая 1
     * @return false — логический 0
     *
     */
    virtual bool read_miso() = 0;

    /**
     * @brief Сформировать один тактовый импульс SCLK.
     *
     * Реализация должна обеспечить корректную
     * последовательность фронтов и временные интервалы
     * согласно требованиям SPI-устройства.
     */
    virtual void pulse_clock() = 0;

    /**
     * @brief Задержка на заданное количество микросекунд.
     *
     * @param us Время задержки в микросекундах.
     *
     */
    virtual void delay_us(unsigned us) = 0;
};

#endif // SPI_BITBANGING_DRIVER_HPP
