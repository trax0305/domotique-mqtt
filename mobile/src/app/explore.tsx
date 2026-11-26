import { ScrollView, StyleSheet, Text, View, useColorScheme } from 'react-native';
import { SafeAreaView } from 'react-native-safe-area-context';

import { BottomTabInset } from '@/constants/theme';

export default function CapteursScreen() {
  const dark = useColorScheme() === 'dark';
  const textColor = dark ? '#F6F7FB' : '#182033';
  const mutedColor = dark ? '#A7B0C1' : '#667085';
  const backgroundColor = dark ? '#101521' : '#F6F8FC';
  const cardColor = dark ? '#1B2030' : '#FFFFFF';
  const borderColor = dark ? '#32384D' : '#E5E9F2';

  return (
    <SafeAreaView style={{ flex: 1, backgroundColor }} edges={['top']}>
      <ScrollView contentContainerStyle={styles.scrollContent}>
        <View style={styles.content}>
          <Text style={[styles.eyebrow, { color: mutedColor }]}>ENVIRONNEMENT</Text>
          <Text style={[styles.title, { color: textColor }]}>Capteurs</Text>
          <Text style={[styles.description, { color: mutedColor }]}>Mesures et alertes de luminosité</Text>
          <View style={[styles.card, { backgroundColor: cardColor, borderColor }]}>
            <Text style={[styles.label, { color: mutedColor }]}>LUMINOSITÉ AMBIANTE</Text>
            <Text style={[styles.measure, { color: textColor }]}>— <Text style={styles.unit}>lux</Text></Text>
            <Text style={[styles.message, { color: mutedColor }]}>En attente des données du capteur Wokwi.</Text>
          </View>
          <View style={[styles.card, { backgroundColor: cardColor, borderColor }]}>
            <Text style={[styles.cardTitle, { color: textColor }]}>Alertes</Text>
            <Text style={[styles.message, { color: mutedColor }]}>Aucun capteur MQTT connecté pour le moment. Les alertes de seuil seront ajoutées lors de l'intégration AWS.</Text>
          </View>
          <Text style={[styles.footer, { color: mutedColor }]}>Cet écran est volontairement sans fausses mesures : les valeurs viendront de l'ESP32.</Text>
        </View>
      </ScrollView>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  scrollContent: { alignItems: 'center', paddingBottom: BottomTabInset + 36 },
  content: { maxWidth: 760, width: '100%', paddingHorizontal: 20, paddingTop: 28, gap: 16 },
  eyebrow: { fontSize: 11, fontWeight: '800', letterSpacing: 1.5 },
  title: { fontSize: 36, fontWeight: '800', marginTop: -6 },
  description: { fontSize: 14, marginTop: -10, marginBottom: 10 },
  card: { borderWidth: 1, borderRadius: 20, padding: 22, gap: 10 },
  label: { fontSize: 11, fontWeight: '800', letterSpacing: 0.8 },
  measure: { fontSize: 46, fontWeight: '800' },
  unit: { fontSize: 16, fontWeight: '500' },
  cardTitle: { fontSize: 19, fontWeight: '800' },
  message: { fontSize: 13, lineHeight: 20 },
  footer: { fontSize: 12, lineHeight: 19 },
});
