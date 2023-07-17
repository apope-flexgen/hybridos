/* eslint-disable import/prefer-default-export */
export const dashboardBoxSx = {
  display: 'flex',
  flexDirection: 'column',
  gap: '20px',
  overflow: 'hidden visible',

  /** hide scrollbar */
  '&::-webkit-scrollbar': { display: 'none' }, // chrome
  'scrollbar-width': 'none', // firefox
};
