export const getRoutes = (siteConfiguration: {
    timezone: string

    configDiff: boolean
    configEdit: boolean
    configHistory: boolean
    siteServerSelection: boolean
}) => {
    const routes = [
        {
            componentName: 'Dashboard',
            icon: 'Dashboard',
            itemName: 'Dashboard',
            path: '/',
        },
        siteConfiguration.configDiff && {
            componentName: 'ConfigDiff',
            icon: 'SiteControls',
            itemName: 'Config Diff',
            path: '/config-diff',
        },
        siteConfiguration.configEdit && {
            componentName: 'ConfigEdit',
            icon: 'Events',
            itemName: 'Config Edit',
            path: '/config-edit',
        },
        siteConfiguration.configHistory && {
            componentName: 'ConfigHistory',
            icon: 'Scheduler',
            itemName: 'Config History',
            path: '/config-history',
        },
        siteConfiguration.siteServerSelection && {
            componentName: 'SiteServerSelection',
            icon: 'Feeders',
            itemName: 'Site Server Selection',
            path: '/site-server-selection',
        },
    ]
    return routes.filter((item) => typeof item !== 'boolean')
}
