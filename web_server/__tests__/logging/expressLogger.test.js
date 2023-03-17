const e = require("express");
const expressLogger = require("../../src/logging/expressLogger")

describe('expressLogger', () => {
    test('express logger IS created', () => {
      req = {}
      res = {}
      next = jest.fn()
      expressLogger(req,res,next);
      expect(next).toHaveBeenCalled()
    });
    test('express logger IS NOT created', () => {
      req = ""
      res = {}
      next = jest.fn()
      expect(expressLogger(req,res,next)).toBeUndefined()
    });
});
