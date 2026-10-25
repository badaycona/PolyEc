#pragma once
#include "person.hpp"
namespace pe {
struct Request { int id=0; Id from=0,to=0; int status=0; }; // pending0/accepted1/fulfilled2/cancelled3
enum class ProposalKind:int { Sale,Loan,Lease,Delivery,Appointment };
enum class ContractStatus:int { Pending,Active,Fulfilled,Rejected,Cancelled,Expired,Defaulted };
struct Proposal { int id=0; ProposalKind kind=ProposalKind::Sale; Id from=0,to=0,author=0,awaiting=0,site=0,source_site=0; Good good=Good::Food; int amount=1,price=1,duration=12,deadline=0; ContractStatus status=ContractStatus::Pending; Money escrow=0; Inventory reserved; };
struct Debt { int id=0,proposal=0; Id lender=0,borrower=0; Money principal=0,interest=0,outstanding=0; int due=0,status=0; };
struct Lease { int id=0,proposal=0; Id landlord=0,tenant=0,site=15; int start=0,end=0; Money rent=0; int status=0; };
struct State; struct Command;
namespace contracts {void propose(State&,const Command&);void respond(State&,const Command&);void counter(State&,const Command&);void fulfill(State&,const Command&);void repay(State&,const Command&);void end_lease(State&,const Command&);void advance(State&);void validate(const State&);Money escrow_total(const State&);Inventory reserved_total(const State&);bool residence(const State&,Id,Id);bool residence_at(const State&,Id,Id,int);}
}
