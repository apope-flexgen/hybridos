import { render } from '@testing-library/react'
import { lightTheme } from '@flexgen/storybook'
import { ThemeProvider } from 'styled-components'

const renderWithThemeWrapper = (Component: any) => {
    return render(
        <ThemeProvider theme={lightTheme}>
            <Component />
        </ThemeProvider>
    )
}

export default renderWithThemeWrapper
