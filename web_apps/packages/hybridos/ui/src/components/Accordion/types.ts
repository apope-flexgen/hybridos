import { ReactNode } from 'react';

export interface AccordionProps {
  children: ReactNode;
  expanded: boolean;
  onExpand: (name: string, isExpanded: boolean) => void;
  name: string;
  deleteText: string;
  onDelete: () => void;
}
