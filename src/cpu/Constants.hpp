#pragma once

/**
 * Constants.hpp
 * 
 * Constantes globais usadas pelos escalonadores
 */

// ✅ CORREÇÃO: Constante de frequência do clock para conversão de ciclos para segundos
// 1 GHz = 1 bilhão de ciclos por segundo (valor realista para CPU moderna)
// Com 1 GHz: 1 ciclo = 1 nanosegundo
// CLOCK_FREQ_HZ: frequency used to convert simulation cycles to seconds.
// Historically set to 1e3 (1 KHz) in error. Use a realistic per-cycle frequency
// for simulation conversions. The intended value is 1 GHz (1e9 Hz), meaning
// 1 cycle = 1 nanosecond. Keep the option to override via compile-time flag
// if needed for quick debugging.
constexpr double CLOCK_FREQ_HZ = 1e9;  // 1.000.000.000 Hz = 1 GHz
