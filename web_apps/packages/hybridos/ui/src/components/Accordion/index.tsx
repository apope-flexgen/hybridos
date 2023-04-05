import { Icon, Typography, MuiButton } from '@flexgen/storybook';
import { Accordion as MuiAccordion, AccordionDetails, AccordionSummary } from '@mui/material';
import {
  AccordionDetailsSX, AccordionSummarySX, ButtonsContainer, MuiAccordionSX,
} from './styles';
import { AccordionProps } from './types';

const Accordion = ({
  children,
  expanded,
  onExpand,
  name,
  deleteText,
  onDelete,
}: AccordionProps) => (
  <MuiAccordion
    TransitionProps={{ unmountOnExit: true }}
    expanded={expanded}
    onChange={(_, isExpanded) => onExpand(name, isExpanded)}
    sx={MuiAccordionSX}
  >
    <AccordionSummary
      sx={AccordionSummarySX}
    >
      <Typography text={name} variant="bodyL" />
      {expanded && <Icon color="primary" src="Edit" />}
    </AccordionSummary>
    <AccordionDetails sx={AccordionDetailsSX}>
      {children}
      <ButtonsContainer>
        <MuiButton
          color="error"
          label={deleteText}
          onClick={onDelete}
          variant="outlined"
        />
      </ButtonsContainer>
    </AccordionDetails>
  </MuiAccordion>
);

export default Accordion;
