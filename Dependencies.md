# Dependencies

## Do
- Check that need to support BIDR var self-observers (in handler assignments not algebraic relations): If not eliminate them during FMU Dependency Processing
- Initialization has order dependency issue:
  - Vars using dds to get initial state (ZC and R) depend on x1(t0) calls on their observees so those vars have to be initialized first
  - dd calls made in Variable::Z1() and R1()
  - By adding a sort_by_type_and_order( vars_NC ) call I do the state vars first which helps
  - Could still have interdependencies among the dd based vars
  - Idea: initialize in dependency DAG order but we can have loops including self-observers
  - Idea: use non-dd ND method for init in a first pass to have something OK in there and then do real init
  - Idea: repeat init until values don't change ???
- Also can have order dependency in advance ops!
- Try phased Observers events for observer advance calls from advance_observer instead of direct calls
  - Allows ad hoc Observers collection for pooled ops and to elim duplication
  - Avoids order dependence of observer updates within each phase
- Does simultaneous observer update need to defer x rep updates to avoid order dependence?
  - If r1 and r2 R vars are interdependent and both getting observer updates in the same phase then yes should defer
    - x_0_ values can be interdependent so 1/2/3 calls aren't enough to avoid order dependency
    - With ND dependency is more complex
  - Use advance_observer_F() to do the deferred assignments
- Should simultaneous requantization also defer all updates (due to ND)?
- Add rep lookup API for propagation (calls from set observees) (maybe p(t)) that is same as
  - q(t) for state variables (add an #ifdef to build with this as x(t) for x propagation)
  - x(t) for other variables
  - Don't forward calls to avoid an extra virtual call layer unless you do the (static cast?) trick to suppress virtual lookup
  - Reason: R1/2/3 can be observees and have a lower order q rep and requantize so state variable calling fmu_set_observees_q() will get their q reps: For now set their q rep to == x rep so not a problem
  - Alternative is to make R1/2/3 q(.) calls return full x state or x state w/o highest order term and just use reduced order "q" internally to set next tE value: Did this fo rnow
  - Can defer for now since state vars use x-based observees
- Variable_Con: advance_connection vs advance_connection_observer ?
  - Use advance_connection_observer from BIDR observer advance also?
  - Call advance_observers from advance_connection_observer?
  - Add advance_ns_observers?
  - Are connection variables acting as state vars or R or ? How do they propagate updates and get propagated updates?

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
- R1/2/3 variables don't act as update firewall so they could have bypass (drill-thru) observees added around them and then not do observer advance calls from their observer advance for higher pooling/efficiency
- Support Event Indicator inAssert=false info: Can EI in an assert alter solution? Like adding event times for those crossings?
- Exploit XML annotations on real-valued (state or not) variables that are constant, linear, quadratic, cubic, ...
- If no output and/or local FMU variables being output could find those with no observers and remove their observees so they never update (for efficiency)
- R variable that only depends on when EIs only changes discretely so could use D variable for efficiency and update firewall
  - Even if R is not declared discrete OCT infers that it is discrete in the when block context
- Add/use x_pre in R vars? Need to trigger observer advance cascade if any (? or just x_0 or x_1?) coeffs chg since x_1 used in dir ders so this may not be worth checking for
- Merge ZC and R observers together in Observers for efficiency ?
  - Adjust or use a custom version of var_sort_index() to put them together
  - R observers need to call phased observers but ZCs have no observers: An if or virtual call eats up some of the performance benefit so use a non-virtual has_observers firewall call in front of that
