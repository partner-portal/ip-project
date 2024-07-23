/*
 * Copyright (c) 2020-2020 Prove & Run S.A.S
 * All Rights Reserved.
 *
 * This software is the confidential and proprietary information of
 * Prove & Run S.A.S ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with Prove & Run S.A.S
 *
 * PROVE & RUN S.A.S MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. PROVE & RUN S.A.S SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
 * MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */

//! Safe, small and convenient error handling over C-style error codes
use alloc::boxed::Box;
use alloc::string::{String, ToString};

/// A simple Result type that embarks `pnc::error::Error`
pub type Result<T> = core::result::Result<T, Error>;

/// An Error type that can be chained and contains only a small string message
#[derive(Debug, Eq, PartialEq)]
pub struct Error {
    msg: String,
    cause: Option<Box<Error>>,
}

impl Error {
    /// Create an Error with a given message
    pub fn msg(msg: &str) -> Self {
        Self {
            msg: msg.to_string(),
            cause: None,
        }
    }

    /// Create a higher level error from a previous one with a new message on top
    pub fn context(self, msg: &str) -> Self {
        Self {
            msg: msg.to_string(),
            cause: Some(Box::new(self)),
        }
    }
}

impl alloc::fmt::Display for Error {
    fn fmt(&self, f: &mut alloc::fmt::Formatter<'_>) -> alloc::fmt::Result {
        writeln!(f, "Error: {}", self.msg)?;
        let mut c = &self.cause;
        while let Some(e) = c {
            writeln!(f, "Cause: {}", e.msg)?;
            c = &e.cause;
        }
        Ok(())
    }
}

impl From<ErrorCode> for Error {
    fn from(errcode: ErrorCode) -> Error {
        Error::from(errcode.code()).context(&errcode.get_context())
    }
}

/// A simple Result type with code that embarks `pnc::error::ErrorCode`
pub type ResultCode<T> = core::result::Result<T, ErrorCode>;

/// An Error type that can be chained and contains a string message and a code
#[derive(Debug)]
pub struct ErrorCode {
    code: i32,
    msg: String,
    cause: Option<Box<ErrorCode>>,
}

impl ErrorCode {
    /// Create an Error with a given message and code
    pub fn new(code: i32, msg: &str) -> Self {
        Self {
            code: code,
            msg: msg.to_string(),
            cause: None,
        }
    }

    /// Create a higher level error from a previous one with a new message and code on top
    pub fn context(self, code: i32, msg: &str) -> Self {
        Self {
            code: code,
            msg: msg.to_string(),
            cause: Some(Box::new(self)),
        }
    }

    /// Return error code
    pub fn code(&self) -> i32 {
        self.code
    }

    /// Return context
    pub fn get_context(&self) -> String {
        self.msg.clone()
    }
}

impl alloc::fmt::Display for ErrorCode {
    fn fmt(&self, f: &mut alloc::fmt::Formatter<'_>) -> alloc::fmt::Result {
        writeln!(f, "Error {} : {}", self.code, self.msg)?;
        let mut c = &self.cause;
        while let Some(e) = c {
            writeln!(f, "Cause: {}", e.msg)?;
            c = &e.cause;
        }
        Ok(())
    }
}
