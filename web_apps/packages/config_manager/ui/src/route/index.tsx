import { FC, createElement } from 'react'
import { NavigateFunction, Route, Routes, useNavigate } from 'react-router-dom'

export interface RouteProps {
    componentName: string
    /**  what path to follow when user clicks on this tab */
    path: string
    /** icon to display on tab */
    icon: string
    /** what name to display on the tab */
    itemName: string
}

export interface RoutesProps {
    routes: Array<RouteProps>
    pageDictionary: any
    currentUser: any
}

const componentFactory = (
    component: any,
    currentUser: any,
    pageName: string,
    navigator: NavigateFunction
) => {
    return createElement(
        // @ts-ignore
        component as FC,
        {
            ...{},
            // @ts-ignore
            pageName,
            currentUser,
            navigator,
        }
    )
}

const AppRoutes = ({ routes, pageDictionary, currentUser }: RoutesProps): JSX.Element => {
    const navigator = useNavigate()
    return (
        <Routes>
            {routes.map(({ componentName, itemName, path, ...rest }: RouteProps, i) => (
                <Route
                    element={componentFactory(
                        pageDictionary[componentName],
                        currentUser,
                        itemName,
                        navigator
                    )}
                    key={i}
                    path={path}
                />
            ))}
        </Routes>
    )
}

export default AppRoutes
