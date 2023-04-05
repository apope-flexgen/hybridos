import { ThemeProvider } from 'styled-components'
import { AppBar, darkTheme, lightTheme, Footer } from '@flexgen/storybook'
import { BrowserRouter as Router } from 'react-router-dom'
import AppRoutes from './../../../route'
import NavigationDrawer from '../navigationDrawer'
import Box from '@mui/material/Box'
import { useState } from 'react'
import FetchErrorModal from '../fetchErrorModal'

const BaseApp = (appInitMock: any, pageDictionary: any, currentUser: any): JSX.Element => {
    const [darkMode, setDarkMode] = useState(false)

    const toggleDarkMode = () => {
        setDarkMode(!darkMode)
    }
    const {
        app: {
            timeZone, // TODO: Fix busted type in AppBar
            appBar: {
                appDisplayName,
                // appIcon, // TODO: Expose this as a prop in AppBar
            },
        },
        routes,
    } = appInitMock

    return (
        <Router>
            <ThemeProvider theme={darkMode ? darkTheme : lightTheme}>
                <Box sx={{ display: 'flex', padding: 0, flex: 1, minHeight: '100vh' }}>
                    <AppBar
                        changeTheme={toggleDarkMode}
                        siteName={appDisplayName}
                        timeZone={timeZone}
                    />
                    <NavigationDrawer routes={routes} />
                    <AppRoutes
                        currentUser={currentUser}
                        pageDictionary={pageDictionary}
                        routes={routes}
                    />
                </Box>
                <FetchErrorModal />
                <Footer softwareName='HybridOS Control' version='Version 11.1.0' />
            </ThemeProvider>
        </Router>
    )
}

export default BaseApp