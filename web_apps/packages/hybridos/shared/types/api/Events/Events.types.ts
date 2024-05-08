import { Severities } from '../../../../ui/node_modules/@flexgen/storybook/dist/types/eventsPageTypes'

export interface EventDataType {
  severity: Severities;
  source: string;
  message: string;
  timestamp: string;
}
