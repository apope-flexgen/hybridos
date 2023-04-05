import { RootStates } from '../../../components/common/fetchErrorModal/types'

export const selectFirstQueryWithFetchError = (state: RootStates) => {
    return Object.values(state).reduce((apiSlice) =>
        Object.values(apiSlice.queries).find((query: any) => query.status === 'rejected')
    )
}
