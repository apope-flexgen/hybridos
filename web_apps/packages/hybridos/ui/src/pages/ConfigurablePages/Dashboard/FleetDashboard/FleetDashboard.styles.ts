/* eslint-disable */
// TODO: fix eslint
export const tableBoxSx = {
  display: 'flex',
  flexDirection: 'column',
  gap: '12px',

  overflow: 'hidden visible',

  /** hide scrollbar */
  '&::-webkit-scrollbar': { display: 'none' }, // chrome
  'scrollbar-width': 'none', // firefox
};
