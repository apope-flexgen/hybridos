import { ThemeType } from '@flexgen/storybook';
import { createTheme, Theme } from '@mui/material/styles';

export const setpointRowStyles = {
  header: {
    row: { '& > *': { borderBottom: 'none' } },
    cell: { paddingLeft: 4, bottomBorder: 'none' },
  },
  innerCell: { paddingBottom: 0, paddingTop: 0 },
  innerBox: { margin: 1 },
  uriTypographyBox: {
    display: 'flex',
    flexDirection: 'row',
    gap: '8px',
    alignItems: 'center',
  },
  deleteButton: { borderBottom: 'none' },
};

export const expandingTableHeaderSx = (theme: ThemeType) => ({
  backgroundColor: theme.fgc.expandingTable.header,
});

export const expandingTableStyles = {
  container: { width: '600px', overflow: 'none' },
  footer: {
    main: { display: 'flex', alignItems: 'center' },
    cell: { borderBottom: 'none' },
  },
};

export function createMuiTheme(theme: ThemeType): Theme {
  return createTheme({
    components: {
      MuiPaper: {
        styleOverrides: {
          root: {
            boxShadow: `${theme.fgb.schedulerConfig.shadow.shadow_01} ${theme.fgc.expandingTable.shadow}, 
                          ${theme.fgb.schedulerConfig.shadow.shadow_02} ${theme.fgc.expandingTable.shadow}, 
                          ${theme.fgb.schedulerConfig.shadow.shadow_03} ${theme.fgc.expandingTable.shadow}`,
          },
        },
      },
      MuiTable: {
        styleOverrides: {
          root: {
            backgroundColor: theme.fgc.expandingTable.background,
          },
        },
      },
    },
  });
}

export const modeListBoxSx = (theme: ThemeType) => ({
  height: '100%',
  padding: theme.fgb.schedulerConfig.sizing.padding,
  display: 'flex',
  flexDirection: 'column',
  gap: '10px',
});

export const modeInfoStyles = {
  box: { flexGrow: 1, padding: '16px' },
  buttonBox: {
    marginLeft: 'auto',
    display: 'flex',
    gap: '16px',
  },
  duplicateURITypo: { marginLeft: 'auto', display: 'flex' },
  textWidth: { width: '150px' },
  selectorWidth: { width: '500px' },
  setpointText: { width: '150px', marginTop: '10px' },
};

export const modeListStyles = {
  modesText: { marginTop: '20px' },
};

export const siteInformationStyles = {
  outterBox: { display: 'flex', flexDirection: 'column' },
  innerBox: { flexDirection: 'column' },
};
