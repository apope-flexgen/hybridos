import { FC, useMemo } from 'react'
import { DrawerTab } from '@flexgen/storybook'
import { useNavigate } from 'react-router-dom'
import { RouteProps } from '../../../../route'
import styled from 'styled-components'

export interface DrawerTabProps extends Omit<RouteProps, 'componentName'> {
    /** whether this tab is currently selected */
    isSelected?: boolean
    /** holds whether the tab is currently open */
    open: boolean
    /** navigate, usually on click  */
    handleClick?: (path: string) => void
    /** whether to show a divider */
    showDivider?: boolean
}

// TODO: Export interface from DrawerTab component so it can be reused here
const NavigationDrawerTab = ({ icon, itemName, open, path }: DrawerTabProps): JSX.Element => {
    const navigate = useNavigate()
    const handleClick = (route: string) => {
        navigate(route)
    }
    return (
        <DrawerTab
            handleClick={(route) => handleClick(route)}
            icon={icon}
            itemName={itemName}
            open={open}
            path={path}
            showDivider={true}
        />
    )
}

export default NavigationDrawerTab
