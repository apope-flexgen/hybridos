import { test, expect } from '@jest/globals'
import ApiQuery from '../services/query/ApiQuery'
import HttpService from '../services/query/HttpService'

// FIXME: put this somewhere else. this is just a placeholder
// since we don't have a real API yet.
const TEST_URL = 'http://localhost:3000/api/v1/items'

test('ApiQuery', () => {
  const apiQuery = new ApiQuery()
  expect(apiQuery).not.toBeNull()
})

test('ApiQuery.getItems', () => {
  const apiQuery = new ApiQuery()
  expect(apiQuery.getItems).not.toBeNull()

  const url = TEST_URL
  const callback = (data) => {
    expect(data).not.toBeNull()
    expect(data).not.toBeUndefined()
    expect(data).not.toBe('')
  }
  apiQuery.getItems(url, callback)
})

test('ApiQuery.postItems', () => {
  const apiQuery = new ApiQuery()
  expect(apiQuery.postItems).not.toBeNull()

  const url = TEST_URL
  const body = {
    name: 'test',
    description: 'test',
  }
  const callback = (data) => {
    expect(data).not.toBeNull()
    expect(data).not.toBeUndefined()
    expect(data).not.toBe('')
  }
  apiQuery.postItems(url, body, callback)
})
