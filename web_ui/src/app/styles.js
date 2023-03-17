import { createTheme, adaptV4Theme } from '@mui/material';

/*
    Describes main theme
    
    How to use:
        import { withStyles } from 'tss-react/mui';
        import { *STYLE_FUNCTION_FOR_COMPONENT* } from '_path_/_to_/styles;

        export default withStyles(*component_name*, STYLE_FUNCTION_FOR_COMPONENT)

        The keys describe the classes, so if you want the root class you set style as:
        <Component className={classes.root} />
    
    Using theme:
        access as an object, so if you want the primary contrastText color: theme.palette.primary.contrastText
*/
export const theme = createTheme(adaptV4Theme({
    palette: {
        primary: {
            main: 'rgba(0, 220, 249, 1.0)', // cyan-ish, includes top banner, checkmark in the X/checkmark pair, button backgrounds            contrastText: '#fff', // text color when inside primary
            dark: 'rgba(0, 162, 184, 1.0)',
            light: 'rgba(0, 220, 249, 0.35)',
            extraLight: 'rgba(0, 220, 249, 0.15)',
            contrastText: '#000', // text color when inside primary
        },
        secondary: {
            // material-ui v4 had this as red
            // but mui v5 changes this to purple
            // setting to default palette.error.main color
            main: '#d32f2f'
        },
        alarm: {
            main: 'rgba(252, 209, 22, 1.0)', // Yellow
            light: 'rgba(252, 209, 22, 0.35)',
            extraLight: 'rgba(252, 209, 22, 0.15)'
        },
        fault: {
            main: 'rgba(252, 3, 3, 1.0)', // Red
            light: 'rgba(252, 3, 3, 0.35)',
            extraLight: 'rgba(252, 3, 3, 0.15)',
            contrastText: '#FFFFFF',
        },
        text: {
            disabled: 'rgba(0, 0, 0, 0.54)', // gray
            primary: 'rgba(0, 0, 0, 0.87)', // black, common text
            secondary: 'rgba(0, 0, 0, 0.54)', // gray, Assets left side menu item
            // disabled: "rgba(0, 0, 0, 0.38)",
        },
        grays: {

        }
    },
    typography: {
        fontSize: 14, // this line was added to match v1.3.1 formatting
        // there was no fontSize here before
        button: {
            // fontWeight: 800, // works
        },
    },
}));

const drawerWidth = 245;

export const STYLES_LAYOUT = () => ({
    root: {
        display: 'flex',
    },
    toolbar: {
        paddingRight: 24, // keep right padding when drawer closed
    },
    toolbarIcon: { // left-pointing chevron that closes menu drawer
        display: 'flex',
        alignItems: 'flex-end',
        flexDirection: 'row',
        justifyContent: 'flex-end',
        padding: '0 8px',
        ...theme.mixins.toolbar,
        transition: '.2s max-width'
    },
    appBar: {
        zIndex: theme.zIndex.drawer -1,
        transition: theme.transitions.create(['width', 'margin'], {
            easing: theme.transitions.easing.sharp,
            duration: theme.transitions.duration.leavingScreen,
        }),
    },
    appBarShift: { // pushes blue bar left side to the right when drawer open
        marginLeft: drawerWidth,
        width: `calc(100% - ${drawerWidth}px)`,
        transition: theme.transitions.create(['width', 'margin'], {
            easing: theme.transitions.easing.sharp,
            duration: theme.transitions.duration.enteringScreen,
        }),
    },
    menuButton: {
        marginLeft: 12,
        marginRight: 36,
    },
    menuButtonHidden: {
        display: 'none',
    },
    title: { // page title in blue bar
        flexGrow: 1,
        fontSize: '1.6em',
    },
    subheader: {
        fontSize: '18px',
        color: 'black',
        letterSpacing: '.5px',
        marginLeft: '10px',
    },
    drawerPaper: { // the "column" that holds the drawer contents
        top: 64,
        height: `calc(100% - 64px)`,
        position: 'fixed',
        whiteSpace: 'nowrap',
        width: drawerWidth,
        overflowX:'hidden',
        transition: theme.transitions.create('width', {
            easing: theme.transitions.easing.sharp,
            duration: theme.transitions.duration.enteringScreen,
        }),
    },
    drawerPaperClose: {
        position: 'fixed',
        overflowX: 'hidden',
        transition: theme.transitions.create('width', {
            easing: theme.transitions.easing.sharp,
            duration: theme.transitions.duration.leavingScreen,
        }),
        width: theme.spacing(7),
        [theme.breakpoints.up('sm')]: {
            width: theme.spacing(9),
        },
    },
    appBarSpacer: theme.mixins.toolbar,
    content: {
        marginLeft: theme.spacing(7),
        flexGrow: 1,
        overflowX: 'hidden',
        padding: theme.spacing(3),
         transition: theme.transitions.create(['width', 'margin'], {
            easing: theme.transitions.easing.sharp,
            duration: theme.transitions.duration.leavingScreen,
        }),
    },
    contentShift: { 
        marginLeft: drawerWidth,
        width: `calc(100% - ${drawerWidth}px)`,
        overflowX: 'hidden',
        padding: theme.spacing(3),
        transition: theme.transitions.create(['width', 'margin'], {
            easing: theme.transitions.easing.sharp,
            duration: theme.transitions.duration.enteringScreen,
        }),
    },
    chartContainer: {
        marginLeft: -22,
    },
    tableContainer: {
        height: 320,
    },
    logo: {
        width: 128,
    },
});

export const STYLES_FLEET_MANAGER_DASHBOARD = () => ({
})

export const STYLES_EVENTS = () => ({
    datepicker: {
        marginRight: '10px'
    },
    loadingDiv: {
        marginTop: '25%',
        marginLeft: '-50px',
        textAlign: 'center'
    },
    eventsFiltering: {
        display: 'block',
        paddingBottom:' 0.75rem',
        width: '100%',
        textAlign: 'center',
        backgroundColor: '#eeeeee',
        boxShadow: '-1px 1px 2px lightgray'
    },
    labelText: {
        display: 'inline',
        marginRight: '20px',
        color: 'rgba(0, 0, 0, 0.87)',
        fontSize: '0.875rem',
        fontWeight: 400
    },
    table: {
        width: '100%',
        borderCollapse: 'collapse'
    },
    row: {
        borderBottom: '1px solid lightgray',
        lineHeight: '1.8em',
        fontSize: 'small',
        fontWeight: 'normal',
    },
    column: {
        padding: '0.75%',
        textAlign: 'left'
    },
    info: {
        backgroundColor: 'rgb(221, 255, 221)',
    },
    status: {
        backgroundColor: 'rgb(221, 255, 221)',
    },
    alarm: {
        backgroundColor: 'gold',
    },
    fault: {
        backgroundColor: 'rgb(255, 221, 221)',
    },
    pageDisplay: {
        textAlign: 'right',
        fontSize: 'small',
        color: 'gray',
        backgroundColor: '#eeeeee',
        marginTop: '10px',
        boxShadow: '-1px 1px 2px lightgray'
    },
    filterButton: {
        padding: 0,
        border: 'none',
        outline: 0,
        fontSize: 'small',
        color: 'gray',
        backgroundColor: '#fafafa',
        cursor: 'pointer'
    },
    arrow: {
        padding: '0 10px',
        display: 'inline-block',
        fontSize: 'x-small',
        fontWeight: 'bold',
        letterSpacing: '-1px',
        color: '#f62772',
        backgroundColor: '#fafafa',
        marginTop: '3px',
        verticalAlign: 'top'
    },
    grayArrow: {
        color: 'lightgray'
    },
    button: {
        color: 'gray',
        marginLeft: '1em',
        backgroundColor: '#e0e0e0',
        padding: '8px 16px',
        fontSize: '0.875rem',
        minWidth: '64px',
        boxSizing: 'border-box',
        minHeight: '36px',
        transition: 'background-color 250ms cubic-bezier(0.4, 0, 0.2, 1) 0ms, box-shadow 250ms cubic-bezier(0.4, 0, 0.2, 1) 0ms, border 250ms cubic-bezier(0.4, 0, 0.2, 1) 0ms',
        fontWeight: 500,
        lineHeight: '1.4em',
        borderRadius: '4px',
        textTransform: 'uppercase',
        '&:hover': {
            backgroundColor: '#d5d5d5'
        }
    }
})

export const STYLES_KEYBOARD = () => ({
    keyboard: {
        display: 'grid',
        boxSizing: 'border-box',
        backgroundColor: '#d1d1d1',
        borderRadius: '5px',
        padding: '1.5%',
        boxShadow: '-3px 3px 5px gray',
        width: '78%',
        height: '250px',
        gridGap: '2.5% 0.6%',
        gridTemplateColumns: 'repeat(73, 0.8%)',
        gridTemplateRows: 'repeat(4, 23%)',
    },
    element: {
        fontSize: 'medium',
        textAlign: 'center',
        backgroundColor: 'white',
        borderRadius: '5px',
        '&:focus': {
            outline: 0
        },
        '&:active': {
            backgroundColor: 'lightgray',
        }
    },
    button: {
        gridColumn: 'auto / span 5'
    },
    buttonTab: {
        gridColumn: 'auto / span 7'
    },
    buttonCapsLock: {
        gridColumn: 'auto / span 9'
    },
    buttonReturn: {
        gridColumn: 'auto / span 8'
    },
    buttonShift: {
        gridColumn: 'auto / span 11'
    }
    
})

export const STYLES_LOGIN_PAGE = () => ({
    toolbarText: {
        fontSize: 'x-large'
    },
    logo: {
        position: 'fixed',
        width: '120px',
        right: '1.5%',
    },
    form: {
        marginLeft: '20%',
        marginTop: '7%'
    },
    wrapper: {
        marginLeft: '30%',
        marginBottom: '3%'
    },
    loadingDiv: {
        marginTop: '18%',
        textAlign: 'center'
    }
})

export const STYLES_ALERT = () => ({
    root: {
        minWidth: '150px',
        paddingLeft: '15px',
        paddingRight: '15px',
        paddingTop: '10px',
        paddingBottom: '10px',
        marginBottom: '20px',
        border: '1px solid transparent',
        borderRadius: '3px',
    },
    success: {
        backgroundColor: '#5cb85c',
        borderColor: '#5cb85c',
        color: '#5cb85c',
    },
    warning: {
        backgroundColor: '#E2A41F',
        borderColor: '#E2A41F',
        color: '#E2A41F',
    },
    danger: {
        backgroundColor: '#d43f3a',
        borderColor: '#d43f3a',
        color: '#d43f3a',
    },
    text: {
        color: '#fff',
    },
    leftText: {
        marginLeft: '4rem',
    },
    card: {
        minWidth: 150,
        marginBottom: 10,
        width: '100%',
    },
});

export const STYLES_ASSET_CARD = () => ({
    card: {
        minWidth: 150,
        marginBottom: 10,
        width: '100%',
    },
    heading: {
        marginBottom: 10,
    },
    root: {
        width: '100%',
        marginTop: theme.spacing(3),
        overflowX: 'auto',
    },
    panelHeading: {
        fontSize: theme.typography.pxToRem(15),
        fontWeight: theme.typography.fontWeightRegular,
    },
    table: {},
    appBar: {
        position: 'fixed',
    },
    flex: {
        flex: 1,
    },
    gridContainer: {
        flexGrow: 1,
    },
    gridItem: {
        alignContent: 'space-between',
    },
    paperRoot: {
        paddingLeft: theme.spacing(2),
        paddingRight: theme.spacing(2),
        [theme.breakpoints.up('sm')] : {
            paddingLeft: theme.spacing(3),
            paddingRight: theme.spacing(3),
        },
        paddingTop: theme.spacing(2),
        paddingBottom: theme.spacing(2),
    },
    radioRoot: {
        display: 'flex',
    },
    formControl: {
        margin: theme.spacing(3),
    },
    group: {
        margin: `${theme.spacing.unit}px 0`,
    },
});

export const STYLES_FEATURES = () => ({
    root: {
        flexGrow: 1,
    },
    loadingDiv: {
        marginTop: '25%',
        marginLeft: '-50px',
        textAlign: 'center',
    },
    card: {
        marginBottom: 10,
        width: '100%',
    },
});

export const STYLES_NUMBERPAD = () => ({
    root: {
        flexGrow: 1,
        maxWidth: 400,
    },
    label: {
        marginBottom: 15,
    },
});

export const STYLES_CUSTOM_KEYBOARD = () => ({
    root: {
        flexGrow: 1,
        maxWidth: 400,
    },
});

export const STYLES_GRID_VIEW = () => ({
    card_header: {
        backgroundColor: theme.palette.primary.extraLight,
        height: '48px'
    },
    scroll: {
        "&::-webkit-scrollbar": {
            width: '6px',
        },
        "&::-webkit-scrollbar-track": {
            backgroundColor: '#ededed',
            borderRadius: '3px'
        },
        "&::-webkit-scrollbar-thumb": {
            backgroundColor: '#d4d4d4',
            borderRadius: '3px'
        }
    },
    iconAlarmActive: {
        color: theme.palette.alarm.main
    },
    iconFaultActive: {
        color: theme.palette.fault.main
    }
});

export const STYLES_SCHEDULER = () => ({

});

export const STYLES_TIMELINE = () => ({

});

export const STYLES_EVENTLIST = () => ({
});

export const STYLES_FORMLIST = () => ({

});
export const STYLES_SCHEDULER_CONFIGURATION = () => ({
    container: {
        width: '100%',
        height: '90%',
    },
    indicator: {
        top: '0px',
    },
});
export const STYLES_TABLE_VIEW = () => ( {
});
export const DENSE_TABLE_ROW = () => ( {
});
export const DENSE_TABLE_CELL = () => ( {
});
export const BATTERY_CELL = () => ( {
});

export const STYLES_UICONFIG = () => ({

});

export const STYLES_MULTIFACTOR_PAGE = () => ({

});

export const STYLES_CONFIG_LIST = () => ({
    subheader: {
        backgroundColor: theme.palette.primary.main,
        fontSize: '14px',
        color: theme.palette.primary.contrastText
    },
})

export const STYLES_TABLE_ROW = () => ({
    root: {
        '&:hover': {
            backgroundColor: theme.palette.primary.extraLight
        }
    }
})

export const STYLES_SOCKET_GRID_BUTTON = () => ({
    root: {
        fontSize: '20px',
        fontWeight: 'bold'
    },
    alarm: {
        backgroundColor: theme.palette.alarm.main,
    },
    fault: {
        backgroundColor: theme.palette.fault.main,
        color: 'white'
    }
})

export const STYLES_USER_ADMIN = () => ({
    subheader: {
        backgroundColor: theme.palette.primary.main,
        fontSize: '14px',
        color: theme.palette.primary.contrastText
    },
})

export const STYLES_THROUGHPUT_DISPLAY = () => ({
})