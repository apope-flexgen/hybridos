import { useEffect, useState } from 'react'
import { useSelector, shallowEqual } from 'react-redux'
import { selectFirstQueryWithFetchError } from '../../../data/selectors/global'
import { RootStates, QueryData, ErrorStatus } from './types'
import ModalError from '@flexgen/storybook/dist/components/Molecules/ModalError/ModalError'

/** Displays modal with an error if any of the API calls fails */
const FetchErrorModal = () => {
    const [errorData, setErrorData] = useState<ErrorStatus | null>(null)
    const queryWithFetchError = useSelector<RootStates, QueryData>(
        selectFirstQueryWithFetchError,
        shallowEqual
    )

    useEffect(() => {
        if (queryWithFetchError) {
            setErrorData(queryWithFetchError.error ?? null)
        }
    }, [queryWithFetchError])

    const closeModal = () => setErrorData(null)

    const render = errorData ? (
        <ModalError
            errorDescription={errorData.data?.errorMessage}
            errorType={errorData.status}
            headerTitle='An error has occurred'
            isOpen={errorData}
            onCloseAction={closeModal}
        />
    ) : (
        <></>
    )

    return render
}

export default FetchErrorModal
