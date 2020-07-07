object DataModule2: TDataModule2
  OldCreateOrder = False
  OnCreate = DataModuleCreate
  OnDestroy = DataModuleDestroy
  Height = 488
  Width = 618
  object SQLiteUniProvider1: TSQLiteUniProvider
    Left = 56
    Top = 48
  end
  object UniConnection1: TUniConnection
    Database = 'byou93'
    Left = 64
    Top = 120
  end
  object UniQuery1: TUniQuery
    Connection = UniConnection1
    FetchRows = 1000
    Left = 56
    Top = 232
  end
  object UniScript1: TUniScript
    Connection = UniConnection1
    Left = 64
    Top = 176
  end
  object NetHTTPClient1: TNetHTTPClient
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    HandleRedirects = True
    AllowCookies = True
    UserAgent = 'Embarcadero URI Client/1.0'
    Left = 512
    Top = 80
  end
  object NetHTTPRequest_login: TNetHTTPRequest
    Asynchronous = True
    ConnectionTimeout = 0
    ResponseTimeout = 0
    OnRequestCompleted = NetHTTPRequest_loginRequestCompleted
    Left = 512
    Top = 144
  end
  object NetHTTPRequest_census: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 0
    ResponseTimeout = 0
    OnRequestCompleted = NetHTTPRequest_censusRequestCompleted
    OnRequestError = NetHTTPRequest_censusRequestError
    Left = 512
    Top = 208
  end
  object UniSQL1: TUniSQL
    Connection = UniConnection1
    Left = 56
    Top = 288
  end
  object UniTable_census: TUniTable
    TableName = 'user_stats'
    SmartFetch.Enabled = True
    SmartFetch.LiveBlock = False
    Connection = UniConnection1
    FetchRows = 500
    Left = 56
    Top = 352
  end
  object NetHTTPRequest_ids: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 512
    Top = 272
  end
  object UniTable_traversal_ids: TUniTable
    Connection = UniConnection1
    Left = 56
    Top = 416
  end
  object NetHTTPRequest_solo: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 0
    ResponseTimeout = 0
    Left = 512
    Top = 344
  end
  object NetHTTPRequest_extracting: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 520
    Top = 416
  end
end
