import { ThemeType } from '@flexgen/storybook';
import { createTheme, SxProps, Theme } from '@mui/material/styles';

export function createButtonBoxSx(theme: ThemeType): SxProps {
  return {
    display: 'flex',
    gap: theme.fgb.editModal.spacing.gap,
    alignItems: 'center',
    paddingTop: theme.fgb.editModal.spacing.padding,
    justifyContent: 'space-between',
  };
}

export function createInitialOptionsBoxSx(theme: ThemeType): SxProps {
  return {
    display: 'flex',
    flexDirection: 'column',
    gap: theme.fgb.editModal.spacing.padding,
    justifyContent: 'flex-start',
  };
}

export function createTitleBoxSx(theme: ThemeType): SxProps {
  return {
    display: 'flex',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingBottom: theme.fgb.editModal.spacing.padding,
  };
}
export function createDialogSx(theme: ThemeType): SxProps {
  return {
    minWidth: '675px',
    backgroundColor: theme.fgc.modal.color.background,
    padding: theme.fgb.modal.spacing.padding,
    borderRadius: theme.fgb.modal.spacing.modalBorderRadius,
  };
}
function createMuiTheme(theme: ThemeType): Theme {
  return createTheme({
    typography: {
      h5: {
        fontFamily: theme.fgb.editModal.fonts.header.fontFamily,
        fontSize: `${theme.fgb.editModal.fonts.header.fontSize}px`,
        fontWeight: theme.fgb.editModal.fonts.header.fontWeight,
        color: theme.fgc.modal.color.text,
      },
      body1: {
        fontFamily: theme.fgb.editModal.fonts.label.fontFamily,
        fontSize: `${theme.fgb.editModal.fonts.label.fontSize}px`,
        fontWeight: theme.fgb.editModal.fonts.label.fontWeight,
        color: theme.fgc.modal.color.text,
      },
    },
  });
}

export default createMuiTheme;
