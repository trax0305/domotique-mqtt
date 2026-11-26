import { Pressable, StyleSheet, Switch, Text, View, useColorScheme } from 'react-native';

import type { LedId, LedState } from '@/types/lighting';

const COLORS = [
  { name: 'Rouge', hex: '#FF5555' },
  { name: 'Orange', hex: '#FFB44D' },
  { name: 'Vert', hex: '#45D6A0' },
  { name: 'Bleu', hex: '#4B9CFF' },
  { name: 'Violet', hex: '#8267FF' },
  { name: 'Blanc', hex: '#FFFFFF' },
] as const;

type Props = {
  led: LedState;
  onPowerChange: (id: LedId, on: boolean) => void;
  onColorChange: (id: LedId, color: string) => void;
  onBrightnessChange: (id: LedId, change: number) => void;
};

export function LedCard({ led, onPowerChange, onColorChange, onBrightnessChange }: Props) {
  const dark = useColorScheme() === 'dark';
  const palette = {
    surface: dark ? '#1B2030' : '#FFFFFF',
    border: dark ? '#32384D' : '#E5E9F2',
    text: dark ? '#F6F7FB' : '#182033',
    secondary: dark ? '#A7B0C1' : '#667085',
    subtle: dark ? '#272E40' : '#F3F5F9',
  };

  return (
    <View style={[styles.card, { backgroundColor: palette.surface, borderColor: palette.border }]}>
      <View style={styles.header}>
        <View style={styles.headerLabel}>
          <View style={[styles.bulb, { backgroundColor: led.on ? led.color : palette.subtle }]}>
            <Text style={[styles.bulbSymbol, { color: led.on ? '#171924' : palette.secondary }]}>✦</Text>
          </View>
          <View>
            <Text style={[styles.name, { color: palette.text }]}>{led.name}</Text>
            <Text style={[styles.description, { color: palette.secondary }]}>{led.room}</Text>
          </View>
        </View>
        <Switch
          accessibilityLabel={`${led.on ? 'Éteindre' : 'Allumer'} ${led.name}`}
          value={led.on}
          onValueChange={(value) => onPowerChange(led.id, value)}
          trackColor={{ false: dark ? '#474E60' : '#D7DCE6', true: '#A99BFF' }}
          thumbColor={led.on ? '#6752DD' : '#F6F7FB'}
        />
      </View>

      <View style={[styles.statusBar, { backgroundColor: palette.subtle }]}>
        <View style={[styles.statusDot, { backgroundColor: led.on ? '#35C68B' : palette.secondary }]} />
        <Text style={[styles.statusText, { color: palette.secondary }]}>
          {led.on ? 'Allumée' : 'Éteinte'} · {led.color.toUpperCase()}
        </Text>
      </View>

      <Text style={[styles.controlTitle, { color: palette.text }]}>Couleur</Text>
      <View style={styles.colors}>
        {COLORS.map((option) => {
          const selected = led.color.toUpperCase() === option.hex.toUpperCase();
          return (
            <Pressable
              key={option.hex}
              onPress={() => onColorChange(led.id, option.hex)}
              accessibilityRole="button"
              accessibilityLabel={`${led.name} : ${option.name}`}
              accessibilityState={{ selected }}
              style={({ pressed }) => [
                styles.swatchButton,
                selected && { borderColor: dark ? '#F6F7FB' : '#49536C' },
                pressed && styles.pressed,
              ]}>
              <View style={[styles.swatch, { backgroundColor: option.hex }]} />
            </Pressable>
          );
        })}
      </View>

      <View style={styles.brightnessHeader}>
        <Text style={[styles.controlTitle, { color: palette.text }]}>Luminosité</Text>
        <Text style={[styles.brightnessValue, { color: palette.text }]}>{led.brightness} %</Text>
      </View>
      <View style={[styles.progressTrack, { backgroundColor: palette.subtle }]}>
        <View style={[styles.progressFill, { width: `${led.brightness}%`, backgroundColor: led.color }]} />
      </View>
      <View style={styles.brightnessControls}>
        <Pressable
          onPress={() => onBrightnessChange(led.id, -10)}
          disabled={led.brightness === 0}
          accessibilityRole="button"
          accessibilityLabel={`Diminuer la luminosité de ${led.name}`}
          style={({ pressed }) => [styles.stepButton, { backgroundColor: palette.subtle }, pressed && styles.pressed, led.brightness === 0 && styles.disabled]}>
          <Text style={[styles.stepText, { color: palette.text }]}>−</Text>
        </Pressable>
        <Text style={[styles.stepHint, { color: palette.secondary }]}>Réglage par 10 %</Text>
        <Pressable
          onPress={() => onBrightnessChange(led.id, 10)}
          disabled={led.brightness === 100}
          accessibilityRole="button"
          accessibilityLabel={`Augmenter la luminosité de ${led.name}`}
          style={({ pressed }) => [styles.stepButton, { backgroundColor: palette.subtle }, pressed && styles.pressed, led.brightness === 100 && styles.disabled]}>
          <Text style={[styles.stepText, { color: palette.text }]}>+</Text>
        </Pressable>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  card: { borderWidth: 1, borderRadius: 22, padding: 20, gap: 12 },
  header: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', gap: 10 },
  headerLabel: { flexDirection: 'row', alignItems: 'center', gap: 12, flexShrink: 1 },
  bulb: { width: 48, height: 48, borderRadius: 16, alignItems: 'center', justifyContent: 'center' },
  bulbSymbol: { fontSize: 25, fontWeight: '700' },
  name: { fontSize: 19, fontWeight: '700' },
  description: { fontSize: 12, marginTop: 3 },
  statusBar: { borderRadius: 11, flexDirection: 'row', paddingVertical: 10, paddingHorizontal: 12, alignItems: 'center', gap: 8 },
  statusDot: { width: 7, height: 7, borderRadius: 4 },
  statusText: { fontSize: 12, fontWeight: '600' },
  controlTitle: { fontSize: 14, fontWeight: '700', marginTop: 2 },
  colors: { flexDirection: 'row', gap: 7, justifyContent: 'space-between', flexWrap: 'wrap' },
  swatchButton: { padding: 3, borderRadius: 50, borderWidth: 2, borderColor: 'transparent' },
  swatch: { width: 31, height: 31, borderRadius: 18, borderWidth: 1, borderColor: '#B9C0CC' },
  pressed: { opacity: 0.6 },
  disabled: { opacity: 0.35 },
  brightnessHeader: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', marginTop: 3 },
  brightnessValue: { fontSize: 15, fontWeight: '700' },
  progressTrack: { height: 9, borderRadius: 7, overflow: 'hidden' },
  progressFill: { height: '100%', borderRadius: 7 },
  brightnessControls: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', marginTop: 2 },
  stepButton: { width: 42, height: 38, borderRadius: 11, alignItems: 'center', justifyContent: 'center' },
  stepText: { fontSize: 24, fontWeight: '500', lineHeight: 28 },
  stepHint: { fontSize: 11 },
});
