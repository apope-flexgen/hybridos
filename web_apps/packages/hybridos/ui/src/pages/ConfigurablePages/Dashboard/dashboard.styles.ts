export const dashboardBoxSx = {
  display: 'flex',
  flexDirection: 'column',
  gap: '12px',
  overflow: 'hidden visible',
  width: '100%',

  /** hide scrollbar */
  '&::-webkit-scrollbar': { display: 'none' }, // chrome
  'scrollbar-width': 'none', // firefox
};

export const titleButtonBoxSx = { display: 'flex', justifyContent: 'space-between', width: '100%' };

export const dashCardContainerSx = {
  height: '100%',
  paddingBottom: '8px',
  display: 'block',
  borderRadius: '8px',
};

export const dashCardHeaderSx = {
  borderTopLeftRadius: '8px',
  borderTopRightRadius: '8px',
};
export const gridContainerSx = { alignItems: 'flex-end', paddingTop: '8px' };

export const dashCardRowSx = {
  height: '100%',
  paddingBottom: '0.25rem',
  paddingTop: '0.25rem',
  borderRadius: '8px',
};
