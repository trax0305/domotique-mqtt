import { Pressable, ScrollView, StyleSheet, Text, View, useColorScheme } from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';

import { LedCard } from '@/components/led-card';
import { BottomTabInset } from '@/constants/theme';
import { useDemoLeds } from '@/hooks/use-demo-leds';

export default function HomeScreen() {
  const dark = useColorScheme() === 'dark';
  const { leds, setPower, setColor, adjustBrightness, setAllPower } = useDemoLeds();
  const activeCount = leds.filter((led) => led.on).length;
  const palette = {
    background: dark ? '#101521' : '#F6F8FC',
    text: dark ? '#F6F7FB' : '#182033',
    secondary: dark ? '#ADB7C9' : '#68758B',
    surface: dark ? '#1B2030' : '#FFFFFF',
    border: dark ? '#32384D' : '#E5E9F2',
  };

  return (
    <SafeAreaView style={[styles.safeArea, { backgroundColor: palette.background }]} edges={['top']}>
      <ScrollView
        style={styles.scrollView}
        contentContainerStyle={styles.scrollContent}
        showsVerticalScrollIndicator={false}>
        <View style={styles.content}>
          <View style={styles.topRow}>
            <View style={styles.titleWrap}>
              <Text style={[styles.eyebrow, { color: palette.secondary }]}>MA MAISON</Text>
              <Text style={[styles.title, { color: palette.text }]}>Éclairage</Text>
              <Text style={[styles.subtitle, { color: palette.secondary }]}>Contrôle de vos LED RGB</Text>
            </View>
            <View style={[styles.demoBadge, { backgroundColor: palette.surface, borderColor: palette.border }]}>
              <View style={styles.demoDot} />
              <Text style={[styles.demoBadgeText, { color: palette.secondary }]}>DÉMO</Text>
            </View>
          </View>

          <View style={styles.summary}>
            <View>
              <Text style={styles.summaryLabel}>LUMIÈRES ALLUMÉES</Text>
              <Text style={styles.summaryNumber}>{activeCount}<Text style={styles.summaryDenominator}> / {leds.length}</Text></Text>
            </View>
            <View style={styles.summaryIconWrap}>
              <Text style={styles.summaryIcon}>✦</Text>
            </View>
          </View>

          <View style={styles.sectionHeader}>
            <View>
              <Text style={[styles.sectionTitle, { color: palette.text }]}>Mes lumières</Text>
              <Text style={[styles.sectionCaption, { color: palette.secondary }]}>Personnalisez chaque LED</Text>
            </View>
          </View>

          <View style={styles.quickActions}>
            <Pressable
              accessibilityRole="button"
              accessibilityLabel="Allumer toutes les LED"
              onPress={() => setAllPower(true)}
              style={({ pressed }) => [styles.primaryButton, pressed && styles.pressed]}>
              <Text style={styles.primaryButtonText}>Tout allumer</Text>
            </Pressable>
            <Pressable
              accessibilityRole="button"
              accessibilityLabel="Éteindre toutes les LED"
              onPress={() => setAllPower(false)}
              style={({ pressed }) => [styles.secondaryButton, { backgroundColor: palette.surface, borderColor: palette.border }, pressed && styles.pressed]}>
              <Text style={[styles.secondaryButtonText, { color: palette.text }]}>Tout éteindre</Text>
            </Pressable>
          </View>

          {leds.map((led) => (
            <LedCard
              key={led.id}
              led={led}
              onPowerChange={setPower}
              onColorChange={setColor}
              onBrightnessChange={adjustBrightness}
            />
          ))}

          <View style={[styles.notice, { backgroundColor: palette.surface, borderColor: palette.border }]}>
            <Text style={[styles.noticeTitle, { color: palette.text }]}>Mode démonstration</Text>
            <Text style={[styles.noticeText, { color: palette.secondary }]}>
              Les commandes modifient uniquement l'état local de l'application. Aucune commande n'est encore envoyée à AWS ou à Wokwi. Les valeurs sont réinitialisées au redémarrage.
            </Text>
          </View>
        </View>
      </ScrollView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  safeArea: { flex: 1 },
  scrollView: { flex: 1 },
  scrollContent: { alignItems: 'center', paddingBottom: BottomTabInset + 36 },
  content: { maxWidth: 760, width: '100%', paddingHorizontal: 20, paddingTop: 24, gap: 16 },
  topRow: { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'flex-start', gap: 12, marginBottom: 4 },
  titleWrap: { flex: 1 },
  eyebrow: { fontSize: 11, fontWeight: '800', letterSpacing: 1.6, marginBottom: 6 },
  title: { fontSize: 36, lineHeight: 43, fontWeight: '800' },
  subtitle: { fontSize: 14, marginTop: 3 },
  demoBadge: { flexDirection: 'row', alignItems: 'center', gap: 5, paddingHorizontal: 10, paddingVertical: 8, borderRadius: 20, borderWidth: 1 },
  demoDot: { width: 7, height: 7, backgroundColor: '#F5AB55', borderRadius: 4 },
  demoBadgeText: { fontSize: 10, fontWeight: '800', letterSpacing: 0.4 },
  summary: { borderRadius: 22, backgroundColor: '#5748B5', paddingHorizontal: 21, paddingVertical: 22, flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center' },
  summaryLabel: { fontSize: 11, fontWeight: '800', letterSpacing: 1, color: '#E5E0FF' },
  summaryNumber: { marginTop: 7, fontSize: 38, lineHeight: 45, color: '#FFFFFF', fontWeight: '800' },
  summaryDenominator: { fontSize: 20, color: '#CDC7FC', fontWeight: '600' },
  summaryIconWrap: { width: 66, height: 66, backgroundColor: '#7768D2', borderRadius: 22, alignItems: 'center', justifyContent: 'center' },
  summaryIcon: { fontSize: 42, lineHeight: 50, color: '#FFFFFF' },
  sectionHeader: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', marginTop: 6 },
  sectionTitle: { fontSize: 22, fontWeight: '800' },
  sectionCaption: { fontSize: 12, marginTop: 4 },
  quickActions: { flexDirection: 'row', gap: 10 },
  primaryButton: { backgroundColor: '#6E5EE8', borderRadius: 12, paddingVertical: 14, alignItems: 'center', flex: 1 },
  primaryButtonText: { color: '#FFFFFF', fontSize: 13, fontWeight: '800' },
  secondaryButton: { borderRadius: 12, paddingVertical: 14, alignItems: 'center', flex: 1, borderWidth: 1 },
  secondaryButtonText: { fontSize: 13, fontWeight: '800' },
  pressed: { opacity: 0.72 },
  notice: { borderWidth: 1, borderRadius: 16, padding: 16, gap: 7, marginTop: 2 },
  noticeTitle: { fontSize: 14, fontWeight: '800' },
  noticeText: { fontSize: 12, lineHeight: 19 },
});
