#pragma once

/**
 * Constants.hpp
 * 
 * Constantes globais usadas pelos escalonadores
 */

// ✅ CORREÇÃO: Constante de frequência do clock para conversão de ciclos para segundos
// 1 GHz = 1 bilhão de ciclos por segundo (valor realista para CPU moderna)
// Com 1 GHz: 1 ciclo = 1 nanosegundo
constexpr double CLOCK_FREQ_HZ = 1e9;  // 1.000.000.000 Hz = 1 GHz
