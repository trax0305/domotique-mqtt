import { useState } from 'react';

import type { LedId, LedState } from '@/types/lighting';

// Données locales pour mettre au point l'interface sans AWS ni Wokwi.
// Elles seront remplacées par les états reçus sur les topics MQTT /state.
const INITIAL_LEDS: LedState[] = [
  { id: 1, name: 'LED 1', room: 'Éclairage principal', on: true, color: '#8267FF', brightness: 70 },
  { id: 2, name: 'LED 2', room: 'Éclairage secondaire', on: false, color: '#FFB44D', brightness: 50 },
];

function clampBrightness(value: number) {
  return Math.max(0, Math.min(100, value));
}

export function useDemoLeds() {
  const [leds, setLeds] = useState<LedState[]>(INITIAL_LEDS);

  function setPower(id: LedId, on: boolean) {
    setLeds((current) => current.map((led) => led.id === id ? { ...led, on } : led));
  }

  function setColor(id: LedId, color: string) {
    // Choisir une couleur allume la LED ; la luminosité existante est conservée.
    setLeds((current) => current.map((led) => led.id === id ? { ...led, color, on: true } : led));
  }

  function adjustBrightness(id: LedId, change: number) {
    setLeds((current) => current.map((led) =>
      led.id === id ? { ...led, brightness: clampBrightness(led.brightness + change) } : led
    ));
  }

  function setAllPower(on: boolean) {
    setLeds((current) => current.map((led) => ({ ...led, on })));
  }

  return { leds, setPower, setColor, adjustBrightness, setAllPower };
}
