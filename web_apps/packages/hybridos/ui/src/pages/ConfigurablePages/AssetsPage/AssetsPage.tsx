import { Tab } from '@flexgen/storybook'
import React, { useState, useEffect } from 'react'
import ConfigurablePagesHOC from 'src/pages/ConfigurablePages/configurablePages.hoc'
import AssetsPageLayout from './assetsPage.layout'
import { AssetsPageProps } from './assetsPage.types'

const AssetsPage = (props: AssetsPageProps) => {
  const [tabComponents, setTabComponents] = useState<React.ReactElement[]>([])
  const [tabValue, setTabValue] = useState('')

  const { componentState, alertState, componentFunctions } = props

  useEffect(() => {
    const updatedTabComponents = Object.entries(componentFunctions).map(
      ([displayGroupID, displayGroup]) => {
        const iconProps = () => {
          const alerts = alertState[displayGroupID]
          if (alerts === undefined) {
            return {}
          }
          if (alerts.faultInformation.length > 0) {
            return {
              icon: 'Fault',
            }
          }
          if (alerts.alarmInformation.length > 0) {
            return {
              icon: 'Alarm',
            }
          }
        }
        return (
          <Tab
            label={displayGroup.displayName}
            value={displayGroupID}
            iconPosition='end'
            key={displayGroupID}
            {...iconProps}
          />
        )
      }
    )
    setTabComponents(updatedTabComponents)
    setTabValue((prev) => {
      if (prev in componentFunctions) {
        return prev
      }
      return Object.keys(componentFunctions)[0]
    })
  }, [componentFunctions, alertState])

  const handleTabChange = (_: React.ChangeEvent<object>, newValue: unknown) => {
    setTabValue(newValue as string)
  }

  return (
    <AssetsPageLayout
      componentFunctions={componentFunctions[tabValue]}
      handleTabChange={handleTabChange}
      tabComponents={tabComponents}
      tabValue={tabValue}
      assetState={componentState}
      alertState={alertState[tabValue]}
    />
  )
}

export default ConfigurablePagesHOC(AssetsPage, 'assetsPage')
