# StarLink-HybridServer

Unreal Engine 5 기반 멀티플레이어 PvE 게임 프로젝트.
커스텀 네트워킹 플러그인 **StarLink**를 통해 데디케이티드 서버와 P2P를 결합한 하이브리드 서버 구조를 구현한다.

NDC21 [SPICA 하이브리드 데디케이티드 서버 구조](https://youtu.be/DCA-b-9j08o?si=5BWsoB1J5bT7-Y0W)에서 영감을 받았다.
대량의 NPC 연산을 클라이언트에 위임하여 서버 부하를 줄이는 것이 핵심 목표이다.

---

## 프로젝트 구조

```
+-- Source/MythicDungeon/            # 게임 소스 코드
|   +-- AI/                          # NPC 행동트리 기반 AI 로직
|   +-- Character/                   # 캐릭터 베이스 및 플레이어/NPC 구현
|   +-- Game/                        # 게임 규칙 및 서버 게임 상태 관리
|   +-- Player/                      # 플레이어 입력 처리 및 상태 복제
+-- Plugins/StarLink/                # 하이브리드 네트워킹 플러그인
    +-- Source/StarLink/
        +-- Public/
        |   +-- Manager/             # 세션/역할 관리 API (서버 측)
        |   +-- RPC/                 # Master - Client 비동기 RPC 컴포넌트 정의
        |   +-- Service/             # 클라이언트 측 네트워크 서비스
        |   +-- Session/             # 세션 라이프사이클 및 클라이언트 연결
        +-- Private/
            +-- Manager/             # 세션/역할 관리 구현
            +-- Network/             # 커스텀 NetDriver/NetConnection 구현
            +-- RPC/                 # RPC 컴포넌트 구현
            +-- Service/             # 네트워크 서비스 구현
            +-- Session/             # 세션/클라이언트 구현
```

---

## StarLink 플러그인

### 핵심 개념

| 용어 | 설명 |
|------|------|
| **Master Server** | 데디케이티드 서버. 세션 생성과 액터 권한 관리 담당 |
| **Host Client** | 세션의 주인 클라이언트. Listen Server 역할로 Peer 접속을 받음 |
| **Peer Client** | 세션 참여 클라이언트. Host에 연결되어 리플리케이션 수신 |
| **Session** | Host 1명 + Peer N명으로 구성된 연결 단위 |

### 아키텍처

```
+-------------------------------------------------+
|              Master Server (Dedi)                |
|                                                  |
|  +------------------+  +------------------+      |
|  | SessionManager   |  | RoleManager      |      |
|  | - 세션 생성/파괴   |  | - 액터 권한 할당   |      |
|  | - 클라이언트 등록  |  | - 액터 권한 반환   |      |
|  +------------------+  +------------------+      |
|                                                  |
|  UStarLinkSubsystem (GameInstanceSubsystem)      |
+---------+-------------------+--------------------+
          |                   |
   +------v------+     +------v------+
   | Client A    |     | Client B    |
   | SLService   |     | SLService   |
   | +- SLClient |     | +- SLClient |
   | |  (Host)   |     | |  (Peer)   |
   | |  NetDriver|     | |  NetDriver|
   | +-----------|     | +-----------|
   +-------------+     +-------------+
```

- **Manager 레이어 (서버):** 게임 로직이 호출하는 API (`USLSessionManager`, `USLRoleManager`)
- **Service 레이어 (클라이언트):** 각 인스턴스에서 실제 동작 (`USLService`, `USLClient`)

### 세션 흐름

```
1. 세션 생성 요청
2. SLRPC_CREATE_HOST              -> Host가 Listen NetDriver 생성
3. SLRPC_CONNECT_PEER (Master)    -> Master가 Peer로 Host에 연결
4. SLRPC_CONNECT_PEER (Peers)     -> 나머지 Peer들이 Host에 연결
5. Completed
```

세션 상태: `Idle` -> `RequestHostClient` -> `ConnectMasterClient` -> `ConnectPeerClients` -> `Completed` / `Failed`

### 액터 권한 이전

**Assign (위임):** Master에서 권한 제거 -> Host에서 권한 부여

```
RoleManager.Assign(Actor, SessionId)
  -> Session.Assign: ClientRPC SLRPC_ASSIGN(GUID)
    -> Host SLService.Assign(GUID)
      -> ISLRoleActor::Host_OnAssign()
```

**Return (반환):** Host에서 권한 제거 -> Master에서 권한 복원

```
RoleManager.Return(Actor)
  -> Session.Return: ClientRPC SLRPC_RETURN(GUID)
    -> Host SLService.Return(GUID)
      -> ISLRoleActor::Host_OnReturn()
```

**HandOver:** 클라이언트 접속 종료 시 할당된 액터를 Master로 자동 반환

### 주요 클래스

#### UStarLinkSubsystem
`UGameInstanceSubsystem` 기반 진입점. `UStarLinkSubsystem::Get(WorldContextObject)`로 접근한다.

#### USLSession
하나의 세션 라이프사이클을 관리한다. Host/Peer 연결 및 Assign/Return RPC 전송을 담당한다.

#### USLClient
NetDriver를 래핑하여 Host 모드(Listen)와 Peer 모드(Connect)로 동작한다.

#### USLRPCHelperComponent

Master Server 와 Client 간의 통신을 지원하는 PlayerController에 부착되는 양방향 비동기 RPC 컴포넌트.

```
[호출 측]                                       [수신 측]
CallClientRPCWithResponse(RPCName, Params)
  |-- RequestId 생성, Promise 저장
  |-- ClientRPC_Request ---------------------->  ClientRPC_Request_Implementation
  |                                                |-- 로직 실행, ResponseData 생성
  |                                                |-- ServerRPC_Response
  |                                       <-----   |
  |-- Promise.SetValue(Response)
  |-- Future.Then() 콜백 실행
```

| 메서드 | 방향 | 설명 |
|--------|------|------|
| `CallServerRPCWithResponse()` | Client -> Server | 서버에 요청 후 `TFuture` 반환 |
| `CallClientRPCWithResponse()` | Server -> Client | 클라이언트에 요청 후 `TFuture` 반환 |

지원 RPC 명령: `SLRPC_CREATE_HOST`, `SLRPC_CONNECT_PEER`, `SLRPC_ASSIGN`, `SLRPC_RETURN`

#### ISLRoleActor

액터가 권한 이전 콜백을 받기 위해 구현하는 인터페이스.

- `Master_OnAssign()` / `Host_OnAssign()` - Assign 시 서버/클라이언트 각각에서 호출
- `Master_OnReturn()` / `Host_OnReturn()` - Return 시 서버/클라이언트 각각에서 호출

---

## 게임 시스템

### AMDCharacterNonPlayer

NPC 캐릭터. `AMDCharacter`를 상속하고 `ISLRoleActor`를 구현하여 StarLink 권한 위임에 대응한다.

**경량 이동 복제 (FRepMoveLite):**
CharacterMovementComponent는 클라이언트에서도 서버와 비슷한 연산을 수행하여 위임 효과가 적었다.
기본 `MovementReplication`을 비활성화하고 경량 구조체로 대체하여 연산량을 크게 줄였다.

```cpp
USTRUCT()
struct FRepMoveLite
{
    UPROPERTY() FVector_NetQuantize10 Pos;
    UPROPERTY() FVector_NetQuantize10 Vel;
    UPROPERTY() uint16 Yaw;
};
```

- **Authority:** `AcceptDistanceSq` 초과 이동 시에만 `RepMove` 갱신
- **Client:** `FMath::VInterpTo`로 보간, CMC SimulatedTick 비활성화

**ISLRoleActor 구현:**

| 콜백 | 역할                           | 동작                          |
|------|------------------------------|-----------------------------|
| `Master_OnAssign()` | Master Server 에서 Assign 시 호출 | AI 정지, CMC Tick off, 복제 off |
| `Host_OnAssign()` | Host Client  에서 Assign 시 호출  | CMC Tick on, AIController Possess, 복제 on                 |
| `Master_OnReturn()` | Master Server 에서 Assign 시 호출 |  CMC Tick on, AIController Possess, 복제 on                |
| `Host_OnReturn()` | Host Client  에서 Return 시 호출  |  AI 정지, CMC Tick off, 복제 off                             |

`PossessedBy()` 시 Host의 `ClientId`에 따라 `ColorIndex`를 설정하여 위임 호스트를 시각적으로 구분한다.

### 캐릭터

- `AMDCharacter` - 베이스 캐릭터
- `AMDCharacterPlayer` - 플레이어 캐릭터 (탑다운 카메라)
- `AMDCharacterNonPlayer` - 논플레이어 캐릭터, 위임/반환에 사용하는 캐릭터

### AI

- `AMDAIController` - BehaviorTree 기반 NPC AI 컨트롤러
- `UBTTask_FindPatrolPos` - 패트롤 위치 탐색 태스크

---

## 콘솔 명령어

`AStarLinkPlayerController`에서 제공:

```
starlink host                                # 세션 생성 (Host 역할 시작)
starlink assign <netguid> <sessionid>        # 액터를 세션에 할당
starlink return <netguid>                    # 액터 권한 반환
starlink batch <start> <end> <sessionid>     # GUID 범위 일괄 할당
```

---

## TroubleShooting

### CharacterMovement 클라이언트 연산량
CMC가 클라이언트에서도 서버 수준 연산을 수행하여 위임 효과가 적었다.
-> `FRepMoveLite` 기반 경량 이동 복제로 해결. (`AMDCharacterNonPlayer` 참조)

### 하이브리드 클라이언트 환경 구성
Host Client는 서버 역할도 하므로 AI/네비게이션을 클라이언트에서 활성화해야 한다.
또한 `WITH_SERVER_CODE` 심볼 문제로 `TargetType.Game`으로 빌드해야 한다.

### 호스트 액터 생성 제약
Host는 `IsNetGUIDAuthority()` 권한이 없어 GUID를 할당할 수 없다. 필요한 액터는 모두 서버에서 생성해야 하며, AIController 등도 `bReplicates = true`로 복제한다.

### 채널 캐시 무효화
액터 반환 시 Game NetDriver의 ActorChannel 캐시에 이전 값이 남아 리플리케이션이 누락될 수 있다.
-> 반환 시 `Channel->Close(EChannelCloseReason::Relevancy)`로 캐시를 초기화하여 해결.