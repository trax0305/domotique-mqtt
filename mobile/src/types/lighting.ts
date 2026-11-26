/** Contrat de données local. Les commandes MQTT utiliseront les mêmes clés. */
export type LedId = 1 | 2;

export type LedState = {
  id: LedId;
  name: string;
  room: string;
  on: boolean;
  color: string; // RGB hexadécimal : #RRGGBB
  brightness: number; // Entier compris entre 0 et 100
};
