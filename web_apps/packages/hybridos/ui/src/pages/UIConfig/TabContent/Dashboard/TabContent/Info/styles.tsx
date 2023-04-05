import styled from 'styled-components';

export const Row = styled.div`
  margin: 16px 0;
  display: flex;
  align-items: baseline;
`;

export const ColumnLeft = styled.div`
  width: 160px;
  min-width: 160px;
`;

export const Item = styled.div`
 display: flex;
 column-gap: 24px;
`;

export const AccordionSX = {
  width: '100%',
};

export const AccordionSummarySX = {
  background: '#F7F8F9',
  display: 'flex',
  justifyContent: 'space-between',
  '& .MuiAccordionSummary-content': {
    display: 'flex',
    justifyContent: 'space-between',
  },
};

export const AccordionDetailsSX = {
  padding: '16px',
  display: 'flex',
  flexDirection: 'column',
  alignItems: 'flex-start',
  rowGap: '24px',
};
