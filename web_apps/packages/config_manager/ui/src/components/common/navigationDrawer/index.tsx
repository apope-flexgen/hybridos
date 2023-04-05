import { useState } from 'react'
import FlexDrawer from '@flexgen/storybook/dist/components/Layout/Drawer/Drawer'
import NavigationDrawerTab from './tab'
import { RouteProps, RoutesProps } from '../../../route'

const NavigationDrawer = ({
    routes,
}: Omit<RoutesProps, 'pageDictionary' | 'currentUser'>): JSX.Element => {
    const [open, setOpen] = useState(true)
    // TODO: It's nicer UX to have drawer open/close on mouseover vs explicitly clicking:
    //  TODO: Should fix in component vs haq
    return (
        <div
        // onMouseEnter={() => setOpen(!open)}
        // onMouseOut={() => setOpen(!open)}
        >
            <FlexDrawer handleOpenClose={() => setOpen(!open)} open={open}>
                {routes.map(({ icon, itemName, path }: RouteProps, i) => (
                    <NavigationDrawerTab
                        icon={icon}
                        itemName={itemName}
                        key={i}
                        open={open}
                        path={path}
                    />
                ))}
            </FlexDrawer>
        </div>
    )
}

export default NavigationDrawer
