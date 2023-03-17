import React, { Component } from 'react';
import AppRoutes from './AppRoutes';

import { ThemeProvider, StyledEngineProvider } from '@mui/material/styles';
import { theme } from './app/styles';
// Not sure if we need two sources of css, Material UI withStyles should be robust enough

class App extends Component {
    // eslint-disable-next-line class-methods-use-this
    render() {
        return (
            <StyledEngineProvider injectFirst>
                <ThemeProvider theme={theme}>
                    <AppRoutes />
                </ThemeProvider>
            </StyledEngineProvider>
        );
    }
}

export default App;
