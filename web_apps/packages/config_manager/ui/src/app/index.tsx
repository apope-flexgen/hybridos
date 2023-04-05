import BaseApp from '../components/common/baseApp'
import {
    ConfigDiff,
    ConfigEdit,
    ConfigHistory,
    Dashboard,
    Home,
    SiteServerSelection,
} from '../pages'
import { useGetInitDataQuery } from '../api/configManager'
import { useState, useEffect } from 'react'
import { getRoutes } from './app-helpers'
import { PageLoadingIndicator } from '@flexgen/storybook'
import { lightTheme } from '@flexgen/storybook'
import { ThemeProvider } from 'styled-components'
import { initData } from '../utils/constants'

const PageDictionary = {
    ConfigDiff: ConfigDiff,
    ConfigEdit: ConfigEdit,
    ConfigHistory: ConfigHistory,
    Dashboard: Dashboard,
    Home: Home,
    SiteServerSelection: SiteServerSelection,
}

const App = (): JSX.Element => {
    const [appInitData, setAppInitData] = useState(initData)

    const [currentUser, setCurrentUser] = useState(undefined)
    const [isLoading, setLoading] = useState<boolean>(true)

    const { data } = useGetInitDataQuery()

    useEffect(() => {
        if (data) {
            const siteConfiguration: any = data

            setAppInitData({
                app: {
                    appName: 'Hybridos Config Manager',
                    timeZone: siteConfiguration.timezone,
                    appBar: {
                        appDisplayName: 'Config Manager',
                        appIcon: 'tbd', // TODO: should be included in FlexGen Component Lib
                    },
                },
                routes: getRoutes(siteConfiguration),
                menuItems: [
                    {
                        children: {},
                        divider: false,
                    },
                ],
            })
            setLoading(false)
        }
    }, [data])

    // TODO: determine how to handle loading
    const LoadingIndicator = () => {
        return (
            <ThemeProvider theme={lightTheme}>
                <PageLoadingIndicator isLoading={isLoading} type='primary' />
            </ThemeProvider>
        )
    }

    // TODO: Update this after configuring login
    const Base = BaseApp(appInitData, PageDictionary, currentUser)

    return isLoading ? LoadingIndicator() : Base
}

export default App
