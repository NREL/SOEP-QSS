# Dependencies

## Testing
- Interdependencies: Eval order for using latest or rhs all "old"
  - Might have triangular dependency structure s.t. can use only updated inputs but is that what CVode does?
    - pre() then changes nature of dependency and QSS doesn't know if pre() is present
  - In Conditional blocks
    if <condition> then
      der(s1) = ... s2 ...
      der(s2) = ... s1 ...
  - Conditional blocks with reinit()
    - Does state in reinit need to be in pre() ? (see BB)
- UpstreamSampler: swi is switching when it shouldn't
- Case600: Test after observers cascade fix
- SimpleHouseDiscreteTime: Test after observers cascade fix
- Firewall example of when discrete firewalls can help
- Issues with classes of Dependency problems/models
- Non-state variables with derivatives ???

## Specs
When performance is verified write up proposed spec changes
- Dependencies
  - Direct dependencies only: No s-c, No EI -> EI
  - Include local variable dependencies (might be active)
  - Fix algorithms "all dependencies shared" behavior
- Annotations
  - Trajectory polynomial order
  - Dependencies within handler expression sep from elsewhere ?

## Agenda: 2022/11/30
- Revised QSS is working as hoped
  - Observees (for setting variable values in FMU) are short-circuited to states and inputs
  - Observers (for signaling updates) short-circuit around passive variables
  - Passive variables can be output via sampled output times
  - Event indicators can depend on other event indicators to handle passive variable short-circuiting (but prefer Dependencies not to s-c)
  - Discrete intermediate variables can be active (firewall) or passive (short-circuited): Can test to see whether active is worth it in most models
  - Updates flow through intermediate variables immediately for consistency
  - Working with current OCT dependencies (at some reduced efficiency)
  - Not performance optimized yet: A number of efficiency updates deferred
- Results
  - Test models are working with current OCT dependencies as planned: Can be more efficient with dependency changes
  - UpstreamSampler: QSS run seems OK (was broken with previous QSS) but CVode run doesn't show sampler activity! (Doesn't work with Buildings 9) No reference solution to check against
  - Don't know if (non-event-generating) min/max are an issue for some models
  - Need to look at handling of non-state real variables with associated derivative variables
- Issues
  - Event indicator to event indicator dependencies in modelDescription.xml are used in two different situations, which interferes with QSS's ability to patch around the lack of direct dependencies:
    1. An intermediate "signaling" variable modified in on EI block that appears in the other EI expression has bee short-circuited out
    2. Something like the EIs share an expression or dependencies
    - QSS could temporarily add dependencies for both meanings for an inefficient hack (not tried yet)
  - Simultaneous events prevent consistent (order-independent) updating with ND
    - Doing deferred updating of states for now to avoid this but is that ideal?
    - Event handler blocks are tricky:
      - State handlers with interdependencies: Capture FMU post-event states in deferred values before overwriting them to do QSS updates/ND
      - See my DepTestR_ss
      - ZC "handlers" are really conditional observers due to s-c, not handlers, so they should be using updated handler values
      - BIDR handlers could be processed after state handlers do deferred updates but should they?
    - Does FMU/PyFMI do sequential updating in such blocks? Depends on "triangular" dependency structure (excluding pre())?
      - If QSS needs to do this it would need to know what dependencies are from pre()
  - Trying x-based observee values for states
    - A bit more accurate but may cause more ND noise
    - Enables simpler/faster code since BIDR and ZC variables are naturally X based: Can fully exploit this if we decide to stay with X-based
  - Zero crossings
    - Need to set handler observee state before FMU event processing to make sure it sees correct pre() values ?
    - If ZC event fires FMU actually sets handler state at t_bump, not tZ: Add post-event correction for this ? Pass t_bump also and have it back correct x_0_ to ZC(tZ) ?
- Proposed OCT/spec changes
  - Dependencies
    - Direct dependencies only: No short-circuiting. Handler dependencies don't "look through" event indicators to their dependencies
    - No extra dependencies (see Issues)
    - Include those for local variables if practical

## Questions for Modelon
- Need just direct dependencies of all (state, local, output, continuous, discrete, input) variables in Dependencies? No s-c and no looking into EI ZC functions from "handled" variables
- How can we detect R vars not marked discrete but that only changing discretely? (when block in equations can chg their functional form to non-constant but we don't know the context of an EI from the XML)
- Event indicators should no longer depend on other event indicators (directly) in Dependencies:
  - Need the direct dependencies of EIs on the variables appearing in their z-c functions
  - Need dependencies on EIs only for variables modified within the if/when block when the event occurs
- Can QSS assume local variable dependencies are listed in Dependencies? This can allow for more efficient output for a sparse collection of output variables

## New Design
- FMU Dependencies processing gets direct dependencies and creates graph over FMU_Variable nodes
- Can we just flag the var type without storing the dependency graph? Will need to traverse it anyway
- Need to know about dependencies to decide whether active or passive BIDR variable:
  - BID: Active if it has any upstream (observer path) state or zc variables so it can serve as update firewall, otherwise passive
  - R: Assume not worth using as firewall for now so passive unless has connections ? Maybe just do all passive for now
    - If only depends on EI(s) (other than when block in equations) and/or discrete variables then it might be effectively discrete: Refine rules for this
- EIs depending on EIs
  - Will want the option to keep active discrete signaling variables but need EI->EI to use/try s-c passive handler variables
  - Continuous R variables don't need to act as firewall generally unless they are connected so most are probably passive (but maybe they are effectively discrete fairly often?) => Need EI <-> EI deps
- Observers:
  - Need to propagate BIDR observer advance out to the BIDR (X-based) observer subgraph (stops when state|input reached): Since these all have only state and input observees doing observer advance on them doesn't affect each other's advance so can do them all as one meta-Obsevers collection: Most efficient to store that as the Observers for BIDR variables
- Using --passive intermediate variables by default for now since a) doing active right to get firewalling requires phased observer updates with tagging to avoid cycles/repeats, and b) OCT is forcing many/most to passive by short-circuiting them in dependencies

## Issues
- Handler:
  - un_bump_time sets ZC's non-handler observees before handler advance to correct for time bump: This is ugly and inefficient: Is this needed and/or can it be done more efficiently?
  - Is rhs supposed to be updated in top order? Is that always possible in non-singular models? What do PyFMI solvers do? Or should it use "old" values on rhs?
    - With ND how can QSS get (new) trajectory using new rhs values without also using old rhs values?
      - Has to set observee state at +/- t_ND so uses all trajectory coefficients and some (state) observees may also be handlers or observers of handlers that are modified in FMU when event fires
  - QSS can't distinguish pre() dependencies that can break dependency DAG cycle
- BIDR vars that are self-observers: Shouldn't have self in computational observees
- Conditional::handler
  - Can we do the observee FMU value setting inside event loop only if FMU detects a crossing (for efficiency)?
- Non-state variables with derivatives:
  - How do these work? Do they have same dependencies or are the deriv's deps implied deps of the base var?
  - Use derivative in R1|2|3 variables?

## Future
- Maybe move an Observers member into the Triggers/_R and Handlers classes: Have to assign the triggers/handlers before using the observers for output
- Why are advance_observer_1 asserts off sometimes?
- Do computational observees before init_1 since it sets observees (does it need to because ND time bumping?)
  - Then maybe restore is_QSS or is_Input check to fmu_set_x (but use it for sampled output)
- Fix for FMU setting handlers at t_bump rather than tZ: Maybe after trajectory is set back up the x_0_ value to what it would be at tZ
- Outputs:
  - Passive vars treated like sample-only outputs
  - Chg local var treatment to use their observees instead of setting all var FMU state
    - But MW says we mostly dump all outputs so more efficient to set whole FMU state for sampled outputs
    - Could even store a set of all the local+passive output var's observees
    - Or maybe set whole mode observee state for each sampled output point (when prob faster than doing redundant observee call for all requested output vars)
- Refine/add logic for when an R variable is active: connected and/or effectively discrete and has upstream state or zc observer ?
- More efficient pre-bump ZC logic
- Observers:
  - Combine R and ZC collections/updates: Need different index order
  - Consider sep deriv arrays so could do all advance_observer phases together after all pooled calls
  - Do observers observers updates as pooled/phased operation: Can we decide when this is more efficient?
- Handlers:
  - What if handler is also an observee of a handler? Can be consistent if deferred updates are used for all (state) handlers
  - Need/use connected_output_handler_ ???
  - Do QSS, R, and OX advance and advance_F before ZCs to reflect that ZC update is really "after" handlers are changed so should use their modified values
  - Set up fixed Handlers collection member of Conditional and mechanism to merge Handlers for ad hoc situations where multiple Conditionals are firing at the same time (and add mechanism to detect that) so that not paying the cost to set up the same ad hoc Handlers over and over (assuming individual ZC events are common)
- Connections: Add support to new design as needed: Handlers need connection updates, ...

## Action
- Add asserts that we never try to set the value of an EI or var that isn't a state or input with the fmu_set_* calls
- Support EI -> EI as implying a handler dependency: EIs need handler_advance()
- Observee now means the vars you set to get the value from the FMU: Only state and input/connection vars: Short circuit out any internal/non-state/algebraic/temporary variables on observee graph paths
  - This is what we were calling observees in prev version
- Observer now means the vars that a var propagates change to when it updates/changes
  - Initially this doesn't s-c any internal vars: Discrete vars are propagation firewall
  - Add option to s-c discrete vars so we can assess its performance impact
  - Later can s-c over non-discrete, non-output-connected internal vars
- Is self-observee now needed and distinct from self-observer ???
- Since QSS var type (R vs R1|2|3) and s-c behavior can depend on var specs the graph operations should probably happen on a free-standing graph sep from the QSS vars
  - Maybe add more specs to FMU_Var including observer & observee pointer/index lists ???
- Enforce that EIs and local vars can't have connections ???
- Chg results output logic to assume we have deps for local and output vars ? Or if no observees use "assume all" logic for results?

## Plan
- The simulation value and directional derivative dependency graph is short-circuited to downstream state and input dependencies (observees) as we need since it isn't correct to set the FMU value of internal variables.
- The update signaling dependency graph is the (not-short-circuited) upstream direct reverse dependencies (observers) so that internal variables can notify connections when they update and can be a firewall when then update but don't change value.
  - This can be refined to treat some internal variables as passive and short-circuit them out of the signaling graph: specifically, continuous internal variables not connected to another FMU.
- To make this approach work we need to return to the plan of having the <Dependencies> section give QSS direct dependencies without any short-circuiting (QSS will do the short-circuiting it needs). Specifically, this means:
  - Variables modified within a conditional block depend on the event indicator but not on variables because they appear in the event indicator's zero-crossing function.
  - Event indicators depend only on the variables appearing within their zero-crossing function.
  - Dependencies on (local and output) internal variables are included and not short-circuited to the state and input variables they depend on.
  - In a scenario like DepTest4 with integer(time) that generates event indicators I think it is still OK to short-circuit out the temporary variable and have dependencies between event indicators.
- Update the SOEP spec once this is working and confirmed to be the best approach

## Do
- Once new design is working refine the event indicator pre-bump logic for more efficiency (don't need to always do the pre-bump):
  - Do an event indicator pass at startup (currently commented out)
  - Mark all event indicators that start at zero as need_ei_init
  - Do a small bump and an event indicator pass when any need_ei_init event indicator has an update that means it will leave zero for the first time
    - Trajectory has non-zero first derivative
    - Clear need_ei_init after successful bump event
    - This may often/usually be just after t_start for most/all such EIs
  - Alternative: do the pre-bump just before a predicted crossing if the ZC is need_ei_init
  - ? What about detected (unpredicted) crossings?
- Check that need to support BIDR var self-observers (in handler assignments not algebraic relations): If not eliminate them during FMU Dependency Processing
- Initialization has order dependency issue:
  - Vars using dds to get initial state (ZC and R) depend on x1(t0) calls on their observees so those vars have to be initialized first
  - dd calls made in Variable::Z1() and R1()
  - By adding a sort_by_type_and_order( vars_NC ) call I do the state vars first which helps
  - Could still have interdependencies among the dd based vars
  - Idea: initialize in dependency DAG order but we can have loops including self-observers
  - Idea: use non-dd ND method for init in a first pass to have something OK in there and then do real init
  - Idea: repeat init until values don't change ???
- Try phased Observers events for observer advance calls from advance_observer instead of direct calls
  - Allows ad hoc Observers collection for pooled ops and to elim duplication
  - Avoids order dependence of observer updates within each phase
- Add rep lookup API for propagation (calls from set observees) (maybe p(t)) that is same as
  - q(t) for state variables (add an #ifdef to build with this as x(t) for x propagation)
  - x(t) for other variables
  - Don't forward calls to avoid an extra virtual call layer unless you do the (static cast?) trick to suppress virtual lookup
  - Reason: R1/2/3 can be observees and have a lower order q rep and requantize so state variable calling fmu_set_observees_q() will get their q reps: For now set their q rep to == x rep so not a problem
  - Alternative is to make R1/2/3 q(.) calls return full x state or x state w/o highest order term and just use reduced order "q" internally to set next tE value: Did this for now
  - Can defer for now since state vars use X-based observees
- Variable_Con: advance_connection vs advance_connection_observer ?
  - Use advance_connection_observer from BIDR observer advance also?
  - Call advance_observers from advance_connection_observer?
  - Are connection variables acting as state vars or R or ? How do they propagate updates and get propagated updates?
- Combine Triggers_R and Triggers_ZC into Triggers_DD and (?) do those requants together (need revised sort index)
- Observers: Combine R and ZC ranges and advance ops into DD (need revised sort index)
- Observers: Consider sep deriv arrays for each order and do one phase call with all those values: Fewer virtual calls but a lot more array storage (but storage reused for ad hoc Observers)
- Do Inputs need deferred updates in advance_discrete? Only if non-Inputs (that might have Input observees) can be part of the same discrete event pass

## Done
- Add observer advance calls in BIDR observer advance ops
- Short-circuit: EI -> EI, V -> temp_* -> EI, der -> der

## Hot
- init/init_observees called twice for R vars
- R vars as a sep category than BID:
  - Need phased init/requant, ...
  - Need phased observer support
  - Future: Maybe add drill-thru dependencies around them (unless we want them to be used as a firewall)
- How to do staged observer updates for BID (only propagate change when they change value)?
  - What if you s-c them but they have a flag for value changed?
    - This adds if conditional to all observer updates but avoids need for phased!!!
    - Only makes sense if phase observer updates is expensive or complex
- Staged observer updates (for BID var update firewalling)
  - How to do them correctly and efficiently?
  - Lose batching benefits for sake of firewalling
  - Without batching interdependent observers don't get correct order-phased updating!!!

## Summary
- New Dependencies section in XML:
  - Variable dependencies without the short-circuiting of the ModelStructure dependencies
    - That short-circuiting can break QSS simulation when variables modified in handler blocks appearing in event indicators are short-circuited out
  - Derivatives appear separately from their state variables as they are separate FMU variables
  - Derivative self-dependencies are elided (but not derivative dependencies on other derivatives, which QSS needs to s-c)
  - Includes temporary variables: Will want to short-circuit those when they are tagged
- Dependency processing
  - Derivative dependencies are assigned to the corresponding QSS state variable
  - See Short-Circuiting below
- QSS changes:
  - Boolean, Integer, Discrete, and Real (BIDR) variables propagate changes to their observers when doing their own observer advance
    - Discrete (BID) variables only propagate if the (discrete) value actually changes, creating an update propagation "firewall"
    - This means that pooled observer advance operations can no longer be parallelized without mutex locks around the observer advance operations. An option would be to make these secondary observer updates "phased" as their own pooled operations in the event queue, which could then be parallelized
  - Non-state
  - By eliminating short-circuiting of handler-modified variables though the event indicators to their dependencies QSS event indicators will now act as a "firewall" against these unnecessary update propagations: handler-modified variables will only update when an event indicator fires (at a zero crossing) because they depend on the conditional object of the event indicator
  - By having local variable dependencies and tracking them in QSS variables QSS can output them efficiently, without needing to update the full FMU state
- Open questions:
  - Should (non-discrete) non-state Real variables propagate their q or x representations? If changes propagate immediately then it should probably be the x representation
  - Is self-observer state derivative sensitivity still present? If so does the option of state variables propagating their x representation still show benefits?
  - Is it worth it to flag QSS state variable dependencies as dependencies of the variable value and/or derivative?
    - If only derivative changes in handler update don't need to get new state value from FMU

## Input
1. Process Dependencies XML section in annotations:
   - Each Element entry has: FMU variable index and a list of its (forward = observee) dependency FMU variable indexes
   - QSS::fmu::dep::Variable holds the FMU index of the observering variable and its list of observee variable FMU indexes
   - QSS::fmu::FMU_Dependencies holds the collection of QSS::fmu::dep::Variable for later processing
   - Derivative observees are short-circuited to the derivatives' observees (as OCT does in ModelStructure)
   - Event Indicator (Zero-Crossing) observees become their associated Conditional object in the QSS variable dependency graph since they only affect the observing variable when the zero crossing event indicator fires
1. Build QSS variables for the FMU variables (FMU_ME::pre_simulate())
   - For the first pass we are building QSS variables for all the (non-fixed|parameter) FMU variables including input, state, local, temporary, and output variables
    - Derivatives are associated with their corresponding state in QSS and not tracked by their own QSS variable
1. Use the FMU variable info from the previous step to revise the FMU dependencies graph with short-circuiting and ...
   - Short-circuit dependencies on derivatives
   - Short-circuit dependencies on X-based variables other than around event indicators when they are needed or act as firewall against update propagation .......???????????????????
1. Bring dependency info into the QSS variables from the FMU variables and dependencies (FMU_ME::pre_simulate())
   - Merge derivative dependencies into the associated state variable (see short-circuiting info below for derivatives)
     - Maybe flag dependencies coming from derivatives separately from those coming from an input (?) or a handler ??????????????????????????????????
   - Discard dependencies on non-QSS (fixed, parameter, ...) variables
   + What about dependencies on inputs ????????????????????????????

## Short-Circuiting
- Dependencies on derivatives: In OCT/Modelica these become dependencies on the derivative's dependencies (short-circuited) not on its associated state variable ```V -> der(.) -> Deps  to  V -> Deps```
- Dependencies between derivatives
- Dependencies between EIs
- Dependencies on event indicators (variables changed in the "handler" block of the event indicator) become dependencies on their associated Conditional in QSS to limit update propagation to actual firing of the event indicator action
- Dependencies on fixed FMU variables can be discarded since they never change. What about variability parameter???
  - Don't think OCT normally has these
  - They would be ignored since have no QSS variable
- Local variables:
  - OCT short-circuits them out of the standard XML dependencies
  - The DepTest model shows that this can break QSS simulations
  - It would be OK to short-circuit local variables not of this type out but it may not be worth it
- Event indicators serve as a "firewall" to prevent observee-triggered update cascades (via their associtate Conditional object) so we never want to short-circuit their dependencies on their observees through the event indicator to its conditional's observers (as OCT does in ModelStructure)

## X-Based, Pass-Thru Dependencies
- Non-state (event indicator and BIDR variables) can change value (not just derivative) discontinuously and don't have the derivative "layer" that it used by QSS to limit update propagation
- These variables should have immediate propagation of changes to their observers: This can be done in 2 ways:
  - Add the "drill-thru" dependencies to the graph: this could significantly enlarge the node degrees but avoids multi-phase updates
  - Do immediate propagation of updates to X-based observers: this requires multi-phase updates at the same event time so probably slower and more complex
- Event indicators as X-based variables should get updated by their observee observer and handler, as well as requantization, updates or they can be too inaccurate for zero crossing prediction
  - This means either immediate update propagation to observing X-based variables or adding their short-circuited "drill-thru" observees one layer beyond their direct observees

## Notes
- Some real-valued non-state variables may only have dependencies on event indicators so may actually only change discretely: these can have simpler discrete representations
- The XML EventIndicators section now has inAssert tags so we could decide to ignore event indicators that only appear in asserts if we are sure they can't change behavior (like adding time steps when they fire): Need to investigate this but for now won't ignore them

## Issues
- Do we have to worry about tunable parameters? causality="parameter" or "calculatedParameter" with variability="tunable"
  - Means constant between external events? Do those come into the FMU via input variables?
  - Would they be retained in the Dependencies section?
  - How would QSS know when they are changed?
- advance_connection_observer() vs advance_connection()?
  - We don't do both for BIDR: Should we?
  - Does this only make sense for state vars?

## Future
- Merge ZC and R observers together in Observers for efficiency ?
  - Adjust or use a custom version of var_sort_index() to put them together
  - R observers need to call phased observers but ZCs have no observers: An if or virtual call eats up some of the performance benefit so use non-virtual observed() firewall call in front of that
- R1/2/3 variables don't act as update firewall so they could have bypass (drill-thru) observees added around them and then not do observer advance calls from their observer advance for higher pooling/efficiency
- Support Event Indicator inAssert=false info: Can EI in an assert alter solution? Like adding event times for those crossings?
- Exploit XML annotations on real-valued (state or not) variables that are constant, linear, quadratic, cubic, ...
- If no output and/or local FMU variables being output could find those with no observers and remove their observees so they never update (for efficiency)
- R variable that only depends on when EIs only changes discretely so could use D variable for efficiency and update firewall
  - Even if R is not declared discrete OCT infers that it is discrete in the when block context
- Add/use x_pre in R vars? Need to trigger observer advance cascade if any (? or just x_0 or x_1?) coeffs chg since x_1 used in dir ders so this may not be worth checking for
