export const dashboardBoxSx = {
  display: 'flex',
  flexDirection: 'column',
  gap: '20px',
  overflow: 'hidden visible',
  width: '100%',

  /** hide scrollbar */
  '&::-webkit-scrollbar': { display: 'none' }, // chrome
  'scrollbar-width': 'none', // firefox
};
