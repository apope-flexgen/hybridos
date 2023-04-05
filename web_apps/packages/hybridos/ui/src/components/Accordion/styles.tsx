import styled from 'styled-components';

export const TextFieldsContainer = styled.div`
  display: flex;
  column-gap: 16px;
`;

export const ButtonsContainer = styled.div`
  display: flex;
  justify-content: flex-end;
  margin-top: 24px;
`;

export const MuiAccordionSX = {
  marginBottom: '8px',
};

export const AccordionSummarySX = {
  background: 'rgba(0, 104, 119, 0.08)',
  display: 'flex',
  justifyContent: 'space-between',
  '& .MuiAccordionSummary-content': {
    display: 'flex',
    justifyContent: 'space-between',
  },
};

export const AccordionDetailsSX = {
  padding: '16px',
};
