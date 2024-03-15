
function addRow() {
  let table = document.getElementById("ExTable"); 
  let nRow = table.rows.length;
  let row = table.insertRow(nRow-1);
  
  let energy = Math.random()*Math.min(heavy[4], heavy[5], heavy[6]);
  let angle = Math.floor(Math.random() * 30) + 10;
  
  row.innerHTML = '<td><input type="text" name="Ex" size="8" value=\"' + energy.toFixed(3) + '\" id=\"Ex' + (nRow-1) + '\" enterkeyhint=\"done\"/></td> \
                   <td><input type="text" name="thetaCM" size="8" value=\"' + angle + '\" id=\"theta' + (nRow-1) + '\" enterkeyhint=\"done\"/></td> \
                   <td></td> \
                   <td></td> \
                   <td></td> \
                   <td></td> \
                   <td></td> \
                   <td></td> \
                   <td></td> \
                   <td></td> \
                   <td></td> \
                   <td></td>';
  
  CalculateEZ();
  CalculateRZ();
  
  //let table2 = document.getElementById("DWBATable"); 
  //row = table2.insertRow(nRow-1);
  //row.innerHTML = '<td id=\"dwba' + (nRow-1) + '\">' + (nRow-2) + '</td> \
  //   <td><input type="text" name="Jpi" size="5" value="3/2+" enterkeyhint="done"/></td> \
  //   <td><input type="text" name="Orb" size="5" value="0d3/2" enterkeyhint="done"/></td>';
}

function deleteRow(){
  let table = document.getElementById("ExTable");
  let nRow = table.rows.length;
  if ( nRow > 3){
     table.deleteRow(nRow-2);
  }
  CalculateEZ();
  CalculateRZ();
  
  //let table2 = document.getElementById("DWBATable");
  //if ( nRow > 3){
  //   table2.deleteRow(nRow-2);
  //}
}
  
let beam = []; //A, Z, Mass, Name, Sn, Sp, Sa
let beamMass;
let yield;
let target= [];
let light=[];
let heavy=[];  //A, Z, Mass, Name, Sn, Sp, Sa

let beamEx;
let BField;
let KEA;
let KE;
let reactionName;

let Qvalue;
let minKEA;

let perpDistant = 11.5; //mm, detector prepdicular distance
let bore = 462.0; // mm
let arrayLen = 50 * 10 + 2 * 9 // SOLARIS 
let detLen = 50; // SOLARIS
let nDet = 10; // SOLARIS
let detGap = 2; // SOLARIS
let pos = []; //1D array, store the 1st pos of detector
let arrayPos = []; // 2D array, store {tip, toe} of each detector

const c = 299.792468; // mm/ns

let beam_k_lab;  
let E_tot_cm ;
let KE_cm ;
let max_Ex ;
let beta ;
let gamma ;

let ez_slope ; // MeV/mm

let alpha ;
let alpha_B ;

let xList =[]; // for E-Z plot
let yList =[]; // for E-Z plot
let ExList = [];
let Zb0List = [];
let rhoList = [];

let ZB0List = [];
let rhoBList = [];

let xRange ;
let yRange = [0, 12];

let zList = []; // for R-Z plot
let rbList = []; // for R-Z plot
let rBList = []; // for R-Z plot

let color = ['rgb(31,119,180)',  // muted blue
             'rgb(255,127,14)',  // safety orange
             'rgb(44,160,44)',  // cooked asparagus green
             'rgb(214,39,40)',  // brick red
             'rgb(148,103,189)',  // muted purple
             'rgb(140,86,75)',  // chestnut brown
             'rgb(277,119,194)',  // raspberry yogurt pink
             'rgb(127,127,127)',  // middle gray
             'rgb(118,189,34)',  // curry yellow-green
             'rgb(23,190,207)'];  // blue-teal

function GetYield(A,Z){
//  let str = 'beamRate.py?A=' + A + '&Z=' +  Z;
//  let client = new XMLHttpRequest();
//  client.onreadystatechange = function() {
//    let haha = client.responseText.split(",");
//    yield = haha[0] 
//  }
//  client.open('GET', str, false);
//  client.send();
// FRIB blocking the request....:(
}

function GetMassFromSym(AZ, id){
  let str = 'massProxy.py?ASym=' + AZ;  
  
  let client = new XMLHttpRequest();
  client.onreadystatechange = function() {
    let mass = client.responseText.split(",");
    if( id == 0 ){
      beam[0] = parseInt(mass[0]);
      beam[1] = parseInt(mass[1]);
      beam[2] = parseFloat(mass[2]);
      beam[3] = AZ;
      beam[4] = parseFloat(mass[4]);
      beam[5] = parseFloat(mass[5]);
      beam[6] = parseFloat(mass[6]);
      GetYield(beam[0], beam[1]);
    }
    if( id == 1 ){
      target[0] = parseInt(mass[0]);
      target[1] = parseInt(mass[1]);
      target[2] = parseFloat(mass[2]);
      target[3] = AZ;
    }
    if( id == 2 ){
      light[0] = parseInt(mass[0]);
      light[1] = parseInt(mass[1]);
      light[2] = parseFloat(mass[2]);
      light[3] = AZ;
    }
  }
  client.open('GET', str, false);
  client.send();

}

function GetMassFromAZ(A,Z){
  let str = 'massProxy.py?A=' + A + '&Z=' + Z;  
  
  let client = new XMLHttpRequest();
  client.onreadystatechange = function() {
    let mass = client.responseText.split(",");
    heavy[2] = parseFloat(mass[2]);
    heavy[3] = mass[3]?.trim();
    heavy[4] = parseFloat(mass[4]);
    heavy[5] = parseFloat(mass[5]);
    heavy[6] = parseFloat(mass[6]);
  }
  client.open('GET', str, false);
  client.send();

}

function GetMass(){

  GetMassFromSym(document.getElementById('beam').value, 0);
  GetMassFromSym(document.getElementById('target').value, 1);
  GetMassFromSym(document.getElementById('light').value, 2);
  
  // console.log(beam);
  // console.log(target);
  // console.log(light);

  beamMass = beam[2];  
  
  heavy[0] = beam[0]+target[0]-light[0];
  heavy[1] = beam[1]+target[1]-light[1];
  GetMassFromAZ(heavy[0], heavy[1]);

  // console.log( heavy);
  
  document.getElementById('heavyName').innerHTML = heavy[3];
  document.getElementById('heavySp').innerHTML = 'Sn: ' + heavy[4] + ' MeV, Sp: ' + heavy[5] + ' MeV, Sa : ' + heavy[6] + ' MeV';
  
  //document.getElementById('beamSp').innerHTML = "haah";
  //document.getElementById('beamYield').innerHTML = "FRIB ultimate yield : " + yield + " pps";
  //document.getElementById('n0').innerHTML = beam[0] + "," + beam[1] + "," + beam[2]
  //document.getElementById('n1').innerHTML = target[0] + "," + target[1] + "," + target[2]
  //document.getElementById('n2').innerHTML = light[0] + "," + light[1] + "," + light[2]
  //document.getElementById('n3').innerHTML = heavy[0] + "," + heavy[1] + "," + heavy[2]
  
}

function CalConstants(){
  
  beamEx = parseFloat(document.getElementById('beamEx').value);
  
  beam[2] = beamMass + beamEx;
  
  BField = parseFloat(document.getElementById('BField').value);
  KEA = document.getElementById('KEA').value;
  KE = KEA * beam[0];
  
  reactionName =  beam[3] +"(" + target[3] + "," + light[3] + ")" + heavy[3] + "@" + KEA + "MeV/u, " + BField.toFixed(2) + " T";

  Qvalue =  - heavy[2] - light[2] + beam[2] + target[2] ;  
  minKEA = (Math.pow(light[2] + heavy[2],2) - Math.pow(beam[2] + target[2],2))/2/target[2]/beam[0];

  document.getElementById('reactionName').innerHTML = reactionName;  
  document.getElementById('minKEA').innerHTML =  "min Beam Energy: " + minKEA.toFixed(3) + " MeV/u";
  document.getElementById('Q-value').innerHTML = Qvalue.toFixed(3);

  beam_k_lab = Math.sqrt(Math.pow(beam[2] + KE,2) - Math.pow(beam[2],2));  
  E_tot_cm = Math.sqrt(Math.pow(target[2] + beam[2],2) + 2*target[2]*KE);
  KE_cm = E_tot_cm - beam[2] - target[2];
  max_Ex = KE_cm + Qvalue;
  beta = beam_k_lab/(beam[2] + target[2] + KE);
  gamma = 1./Math.sqrt(1-beta*beta);
  
  ez_slope = Math.abs(BField) * c * light[1]*beta/2/Math.PI/1000; // MeV/mm
  alpha = ez_slope/beta;
  //alpha_B = alpha * heavy[1]/light[1];
  alpha_B = Math.abs(BField) * c * heavy[1]*beta/2/Math.PI/1000 / beta;

}

function CalArrayPos(){
  let helios = document.getElementById('HELIOS').checked;

  let haha = parseFloat(document.getElementById('posArray').value);
  arrayPos = [];
  for( let i = 0; i < nDet; i++){
    let kaka = [];
    if( helios == true ){
      if( haha < 0 ){
        kaka.push(haha - pos[i] - detLen );
        kaka.push(haha - pos[i]);
      }else{        
        kaka.push(haha + pos[i] );
        kaka.push(haha + pos[i] + detLen );
      }
    }else{
      if( haha < 0 ){
        kaka.push(haha - (i+1) * detLen - i * detGap);
        kaka.push(haha - (i) * detLen - i * detGap);
      }else{
        kaka.push(haha + (i) * detLen + i * detGap);
        kaka.push(haha + (i+1) * detLen + i * detGap);
      }
    }
    arrayPos.push(kaka);
  }
}

function SetSSType(){
  let helios = document.getElementById('HELIOS').checked;
  let solaris = document.getElementById('SOLARIS').checked;
  let iss = document.getElementById('ISS').checked;

  if ( helios == true ) {
    perpDistant = 11.5;
    detGap = 5;
    detLen = 50;
    nDet = 6;
    pos = [];
    pos.push(.0);
    pos.push(58.6);
    pos.push(117.9);
    pos.push(176.8);
    pos.push(235.8);
    pos.push(294.0);
  }
  if ( solaris == true ) {
    perpDistant = 11.5;
    detGap = 5;
    detLen = 50;
    nDet = 10;
  }
  if ( iss == true ) {
    perpDistant = 28.75;
    detGap = 0.5;
    detLen = 125;
    nDet = 4;
  }
  arrayLen = detLen * nDet + detGap * (nDet-1);
  
  if( helios ) arrayLen = pos[5] + detLen;

  CalArrayPos();
  //document.getElementById('n1').innerHTML = perpDistant;
}

function CalculateEZ(){
  
  let tableEx = document.getElementById("ExTable");
  let nRow = tableEx.rows.length;
  
  xList = [];
  yList = [];
  ExList = [];
  Zb0List = [];
  rhoList = [];
  ZB0List = [];
  rhoBList = [];
  
  //alert("CalculateEZ called, nRow = " + nRow);
  
  for( let i = 1; i < nRow-1; i++){
    let Ex = parseFloat(document.getElementById("Ex" + i).value);
    let theta = parseFloat(document.getElementById("theta" + i).value);
    
    ExList.push(Ex);
    //alert( i, ", Ex : " + Ex);
    
    let haha1 = E_tot_cm*E_tot_cm - Math.pow(heavy[2] + Ex + light[2],2);
    let haha2 = E_tot_cm*E_tot_cm - Math.pow(heavy[2] + Ex - light[2],2);
    let k_cm = Math.sqrt(haha1*haha2)/2/E_tot_cm;
    
    let cs = Math.cos(theta*Math.PI/180.);
    let ss = Math.sin(theta*Math.PI/180.);
    let qb = Math.sqrt(light[2]*light[2]+k_cm*k_cm);
    
    let Eb = gamma * qb - gamma * beta * k_cm * cs - light[2]; 
    
    let Zb0 = (gamma*beta* qb - gamma * k_cm * cs )/alpha; //mm
    Zb0List.push(Zb0);
    
    let rho = k_cm * ss/c/light[1]/Math.abs(BField) * 1000; // mm
    rhoList.push(rho);
    let Zb = Zb0 * (1- Math.asin(perpDistant/rho/2)/Math.PI);
    
    let thetaLab = 180 - Math.atan2(k_cm * ss, gamma * (beta * qb - k_cm * cs)) * 180/Math.PI;
    
    let Tcyc = (light[2] + Eb)/alpha/c;
    
    tableEx.rows[i].cells[2].innerHTML = Eb.toFixed(3);
    tableEx.rows[i].cells[3].innerHTML = Zb0.toFixed(1);
    tableEx.rows[i].cells[4].innerHTML = Zb.toFixed(1);
    tableEx.rows[i].cells[5].innerHTML = (2*rho).toFixed(1);
    tableEx.rows[i].cells[6].innerHTML = thetaLab.toFixed(2);
    tableEx.rows[i].cells[7].innerHTML = Tcyc.toFixed(2);
    
    let qB = Math.sqrt(heavy[2]*heavy[2]+k_cm*k_cm);
    let EB =  gamma * qB + gamma * beta * k_cm * cs - heavy[2]; 
    let thetaLab_B = Math.atan2(-k_cm * ss, gamma * (beta * qB + k_cm * cs)) * 180/Math.PI;
    let ZB0 = (gamma*beta* qB + gamma * k_cm * cs )/alpha_B; //mm
    let rhoB = k_cm * ss/c/heavy[1]/Math.abs(BField) * 1000; // mm
    ZB0List.push(ZB0);
    rhoBList.push(rhoB);
    
    tableEx.rows[i].cells[8].innerHTML = EB.toFixed(3);
    tableEx.rows[i].cells[9].innerHTML = thetaLab_B.toFixed(2);
    tableEx.rows[i].cells[10].innerHTML = (ZB0/2).toFixed(2);
    tableEx.rows[i].cells[11].innerHTML = (2*rhoB).toFixed(2);

    let xxx = [];
    let yyy = [];
    
    for( let j = 0; j < 100 ; j++){
      let thetaCM = Math.PI/Math.log10(100) * Math.log10(100/(100-j)) ;
      let temp =  Math.PI * ez_slope / beta / k_cm * perpDistant / Math.sin(thetaCM); // perpDistant / 2/ rho(thetaCM)
      if( !isFinite(temp) ) continue;
      let pxTemp = 1. /alpha * (gamma * beta * qb - gamma * k_cm * Math.cos(thetaCM)) * (1 - Math.asin(temp)/Math.PI) ;
      let pyTemp = gamma * qb - light[2] - gamma * beta * k_cm * Math.cos(thetaCM);
      if( !isNaN(pxTemp) && !isNaN(pyTemp) ){
        xxx.push(pxTemp);
        yyy.push(pyTemp);
      }
    };
    
    xList.push(xxx);
    yList.push(yyy);
    
  };
  
  PlotEZ();
  AdjustRangeEZ();
  
}

function PlotEZ(){

  SetSSType();
  Plotly.purge("Plot_EZ");

  let nEx = xList.length;  
  let data = [];
  
  for( let i = 0; i < nEx; i++){  
    let kaka = color[i%10];  
    let newData = {
      x : xList[i], 
      y : yList[i], 
      mode:"lines", 
      type:"scatter", 
      name:"Ex="+ExList[i],
      marker : { color : kaka}
    }
    data.push(newData);
  }
  
  let haha = parseFloat(document.getElementById('posArrayRange').value);
  let xStart = (haha < 0 ? haha - arrayLen - 100 : haha - 100);
  let xEnd = (haha < 0 ? haha + 100: haha + arrayLen + 100);
  
  xRange = [xStart, xEnd];
  //document.getElementById('n0').innerHTML = xRange;
  
  let haha2 = parseFloat(document.getElementById('eRangeSlider').value);
  yRange = [0, haha2];

  let layout = {
    xaxis: {range: xRange, title: { text : "Z [mm]", standoff : 1}, mirror : "allticks", linewidth : "1"},
    yaxis: {range: yRange, title: "Energy [MeV]" , mirror : "allticks", linewidth : "1"},  
    title: reactionName,
    dragmode : "pan",
    margin: { l: 40, r: 40, b : 60, t : 40},
    legend: {yanchor:"top", xanchor:"left", x:"0.01",y:"0.99"  }
  };
  
  
  Plotly.newPlot( "Plot_EZ", data, layout, {responsive: true});

  PlotThetaCMLine(document.getElementById('thetaCM').value);
  PlotBore();

  PlotRZ();
}

function PlotThetaCMLine(thetaCM){
  let cs = Math.cos(thetaCM * Math.PI /180);
  let ss = Math.sin(thetaCM * Math.PI /180);
  
  let zzList = [];
  let eList = [];
  
  for( let z = -2000; z < 2000; z+=30){
    zzList.push(z);
    
    let kaka = Math.pow(gamma * ss,2);
    let a1 = light[2]*light[2]*(1-kaka);
    let haha = (cs*Math.sqrt(alpha*alpha*z*z + a1) - kaka * ez_slope * z)/(1-kaka)- light[2];
    eList.push(haha);
  }
  
  Plotly.addTraces("Plot_EZ", {x : zzList, 
                               y: eList, 
                               name:"thetaCM=" + thetaCM , 
                               marker : { color : 'rgb(100,100,100)'}, 
                               line : {dash : 'solid', width : 1 }
                               }
                    , 0);
}

function PlotBore(){
  let zzList = [];
  let eList = [];
  
  for( let z = -2000; z < 2000; z+=30){
    zzList.push(z);
    
    let haha = Math.sqrt((z*z+Math.PI*Math.PI*bore*bore)*alpha*alpha + light[2]*light[2]) - light[2];
    eList.push(haha);
  }
  
  Plotly.addTraces("Plot_EZ", {x : zzList, y: eList, name:"Bore", marker : { color : 'rgb(200,200, 200)'}  }, 0);
}

function CalculateRZ(){
  // this rquire ZB0List and rhoBList from CalculateEZ();
  
  zList = [];
  rbList = [];
  rBList = [];
  
  for( let z = -2000; z < 4000; z += 2 ) zList.push(z);
  
  // cal the heavy recoil first
  for( let i = 0; i < ZB0List.length; i++){
    let rrr = []
    for( let j = 0; j < zList.length; j++){
      if( zList[j] < 0 ){
        rrr.push(NaN);
      }else{
        rrr.push(2*rhoBList[i] *Math.abs( Math.sin(zList[j] * Math.PI / ZB0List[i])));
      }
    }
    rBList.push(rrr);
  }
  
  // cal the light recoil first
  for( let i = 0; i < Zb0List.length; i++){
    let rrr = []
    for( let j = 0; j < zList.length; j++){
      if( target[0] > light[0] ){
        if( zList[j] > 0 ){
          rrr.push(NaN);
        }else{
          rrr.push(2*rhoList[i] *Math.abs( Math.sin(zList[j] * Math.PI / Zb0List[i])));
        }
      }else{
        if( zList[j] < 0 ){
          rrr.push(NaN);
        }else{
          rrr.push(2*rhoList[i] *Math.abs( Math.sin(zList[j] * Math.PI / Zb0List[i])));
        }
      }
    }
    rbList.push(rrr);
  }
  
  PlotRZ();
  AdjustRecoilPos();
}

function PlotRZ(){
  Plotly.purge("Plot_RZ");
  
  let data = [];
  let nEx = ExList.length;
  
  for(let i = 0 ; i < nEx; i++ ){
    let kaka = color[i%10];
    let newData = {
      x : zList,
      y : rBList[i], 
      mode : "lines",
      type : "scatter", 
      name : "Ex="+ExList[i] + ",theta=" + document.getElementById('theta'+(i+1)).value,
      marker : { color : kaka} 
    }
    data.push(newData);
  }
  for(let i = 0 ; i < nEx; i++ ){
    let kaka = color[i%10];
    let newData = {
      x : zList,
      y : rbList[i], 
      mode : "lines",
      line : {dash : 'dashdot', width : 1 },
      type : "scatter", 
      name : "Ex="+ExList[i] + ",theta=" + document.getElementById('theta'+(i+1)).value,
      marker : { color : kaka} 
    }
    data.push(newData);
  }
  
  let xxx = [parseInt(document.getElementById('zRange1').value), parseInt(document.getElementById('zRange2').value)];
  let yyy = [0, parseInt(document.getElementById('rRange').value)];
  
  let layout = {
    xaxis: {range: xxx, title: { text : "Z [mm]", standoff : 1}, mirror : "allticks", linewidth : "1"},
    yaxis: {range: yyy, title: "R [mm]" , mirror : "allticks", linewidth : "1"},  
    title: reactionName,
    showlegend : true,
    dragmode : "pan",
    margin: { l: 40, r: 40, b : 60, t : 40},
    legend: { yanchor:"top", xanchor:"left", x:"0.01",y:"0.99"}
  };
  
  Plotly.newPlot( "Plot_RZ", data, layout, {responsive: true});
  
}

function CalZ(theta, k_cm, qb){
  let cs = Math.cos(theta*Math.PI/180.);
  let ss = Math.sin(theta*Math.PI/180.);
    
  let Zb0 = (gamma*beta* qb - gamma * k_cm * cs )/alpha; //mm
  let rho = k_cm * ss / c/ light[1]/ Math.abs(BField) * 1000; // mm
     
  return  Zb0 * ( 1 - Math.sin( perpDistant/2/rho ) / Math.PI );
}

function SearchThetaCM(Z, thetaMin, k_cm, qb){
  
  let step = 0.01;
  
  for( let yy = thetaMin; yy < 180; yy += step){
    if( yy > 12 ) step = 0.1;
    if( yy > 20 ) step = 0.5;
    if( yy > 40 ) step = 1.0;
    let z1 = CalZ(yy, k_cm, qb);
    let z2 = CalZ(yy+step, k_cm, qb);
    //console.log(Z, ", ", yy.toFixed(3), ", ", z1.toFixed(3), ", ", z2.toFixed(3), ", ", (z2-z1).toFixed(3) );
    if( z1 > z2 ){ /// in the bending range
      continue;
    }else{
      if( Z < z1 ){
      //  return yy;
        break;
      }
      if( z1<= Z && Z <= z2 ){ // do a linear approximation
        return (Z-z1)/(z2-z1)*step + yy;
      }
    }
  }
  return NaN;
}

function CalThetaCM(){
  let ex = parseFloat(document.getElementById('Ex0').value);
  let angGate = parseFloat(document.getElementById('thetaCMGate').value);
  let xGate = parseFloat(document.getElementById('XGate').value);

  SetSSType();
  
  let haha1 = E_tot_cm*E_tot_cm - Math.pow(heavy[2] + ex + light[2],2);
  let haha2 = E_tot_cm*E_tot_cm - Math.pow(heavy[2] + ex - light[2],2);
  let k_cm = Math.sqrt(haha1*haha2)/2/E_tot_cm;
  let qb = Math.sqrt(light[2]*light[2]+k_cm*k_cm);
  
  let table = document.getElementById('thetaCMTable');
  let nRow = table.rows.length;
  if( nRow > 1 ){
    for( let i = nRow-1; i > 0; i -- ){
      table.deleteRow(i);
    }
  }

  for( let i = 0; i < arrayPos.length; i++){
    let row = table.insertRow(i+1);
    row.insertCell().innerHTML = i;
    
    let p1 = (arrayPos[i][0] + detLen * (100. - xGate)/200.);
    let p2 = (arrayPos[i][1] - detLen * (100. - xGate)/200.);
    
    row.insertCell().innerHTML = arrayPos[i][0].toFixed(1) + "(" + p1.toFixed(1)  + ")";
    row.insertCell().innerHTML = arrayPos[i][1].toFixed(1) + "(" + p2.toFixed(1)  + ")";
    
    ///Search thetaCM for Z
    let a1 = SearchThetaCM(p1, angGate, k_cm, qb);
    let a2 = SearchThetaCM(p2, angGate, k_cm, qb);
    row.insertCell().innerHTML = a1.toFixed(2);
    row.insertCell().innerHTML = a2.toFixed(2);
    
    let am = (a2+a1)/2;
    let da = Math.abs(a2-a1);
    
    row.insertCell().innerHTML = am.toFixed(2);
    row.insertCell().innerHTML = da.toFixed(2);
    row.insertCell().innerHTML = (Math.sin(am*Math.PI/180) * da * Math.PI/180).toExponential(2);
    
  }
  
}

document.getElementById('beam').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      GetMass();
      CalConstants();
    }
  }, false
);

document.getElementById('beamEx').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      CalConstants();
    }
  }, false
);

document.getElementById('target').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      GetMass();
      CalConstants();
    }
  }, false
);

document.getElementById('light').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      GetMass();
      CalConstants();
    }
  }, false
);

document.getElementById('BField').addEventListener('keypress', 
  function(e){
    //document.getElementById('n0').innerHTML = e.keyCode;
    if(e.keyCode == 13){
      document.getElementById('BRange').value = this.value;
      CalConstants();
      CalculateEZ();
      CalculateRZ();
      CalThetaCM();
    }
  }, false
);

document.getElementById('BRange').oninput = function(){
  document.getElementById('BField').value = this.value;
  CalConstants();
  CalculateEZ();
  CalculateRZ();
  AdjustRangeEZ();
  CalThetaCM();
}

document.getElementById('KEA').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      document.getElementById('KEARange').value = this.value;
      CalConstants();
      CalculateEZ();
      CalculateRZ();
      CalThetaCM();
    }
  }, false
);

document.getElementById('KEARange').oninput = function(){
  document.getElementById('KEA').value = this.value;
  CalConstants();
  CalculateEZ();
  CalculateRZ();
  CalThetaCM();
}

document.getElementById('thetaCM').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      document.getElementById('thetaCMRange').value = this.value;
      CalConstants();
      CalculateEZ();
      AdjustRecoilPos();
    }
  }, false
);

document.getElementById('thetaCMRange').oninput = function(){
  document.getElementById('thetaCM').value = this.value;
  CalConstants();
  CalculateEZ();
  AdjustRecoilPos();
}

function AdjustRangeEZ(){
  let haha = parseFloat(document.getElementById('posArray').value);
  let xStart = (haha < 0 ? haha - arrayLen - 100 : haha - 100);
  let xEnd = (haha < 0 ? haha + 100: haha + arrayLen + 100);
  
  SetSSType();
  
  xRange = [xStart, xEnd];
  yRange = [0, parseFloat(document.getElementById('eRangeSlider').value)];
  
  //document.getElementById('n0').innerHTML = pos;
  //document.getElementById('n2').innerHTML = arrayLen;
  //document.getElementById('n1').innerHTML = xRange;

  let shapeArray = [];
  for( let i = 0; i < nDet; i++){
    let newBlock = { 
      type: 'rect', 
      xref: 'x', 
      yref: 'paper', 
      x0 : arrayPos[i][0], 
      x1 : arrayPos[i][1], 
      y0 : 0,
      y1 : 1,
      fillcolor : '#9999FF',
      opacity : 0.1,
      line : { width : 0} }
    shapeArray.push(newBlock);
  }
  
  let update = {
    'xaxis.range' : xRange,
    'yaxis.range' : yRange,
    'shapes' : shapeArray
  }
  Plotly.relayout("Plot_EZ", update);
}

document.getElementById('posArray').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      document.getElementById('posArrayRange').value = this.value;
      AdjustRangeEZ();
      AdjustRecoilPos();
      CalThetaCM();
    }
  }, false
);

document.getElementById('posArrayRange').oninput = function(){
  document.getElementById('posArray').value = this.value;
  AdjustRangeEZ();
  AdjustRecoilPos();
  CalThetaCM();
}

function AdjustRecoilPos(){
  let pos1 = parseInt(document.getElementById('posRecoil').value);
  let pos2 = pos1+10;

  let radius1 = parseInt(document.getElementById('innerRecoil').value);
  let radius2 = parseInt(document.getElementById('outterRecoil').value);

  //document.getElementById('n0').innerHTML = pos1;

  let shapeRecoil = {
    type: 'rect', 
      xref: 'x', 
      yref: 'y', 
      x0 : pos1, 
      x1 : pos2, 
      y0 : radius1,
      y1 : radius2,
      fillcolor : '#FF0000',
      opacity : 0.4,
      line : { width : 0} 
  };
  
  let totalShape = [];
  for( let i = 0; i < nDet; i++){
    let newhaha = {
      type: 'rect', 
      xref: 'x', 
      yref: 'y', 
      x0 : arrayPos[i][0], 
      x1 : arrayPos[i][1], 
      y0 : 0,
      y1 : perpDistant,
      fillcolor : '#9999FF',
      opacity : 0.1,
      line : { width : 0} };
    totalShape.push(newhaha);
  }
  totalShape.push(shapeRecoil);
  
  let update = {
    'shapes' : totalShape
  }
  Plotly.relayout("Plot_RZ", update);
  
}

document.getElementById('posRecoil').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      document.getElementById('posRecoilRange').value = this.value;
      AdjustRecoilPos();
    }
  }, false
);

document.getElementById('posRecoilRange').oninput = function(){
  document.getElementById('posRecoil').value = this.value;
  AdjustRecoilPos();
}

document.getElementById('innerRecoil').addEventListener('keypress',
  function(e){
    if(e.keyCode == 13){
      AdjustRecoilPos();
    }
  }, false
);

document.getElementById('outterRecoil').addEventListener('keypress',
  function(e){
    if(e.keyCode == 13){
      AdjustRecoilPos();
    }
  }, false
);

function AdjustRangeRZ(){
  
  let zmin = parseInt(document.getElementById('zRange1').value);
  let zmax = parseInt(document.getElementById('zRange2').value);
  let rmax = parseInt(document.getElementById('rRange').value);
  
  let update = {
    'xaxis.range' : [zmin, zmax],
    'yaxis.range' : [0, rmax]
  }
  Plotly.relayout("Plot_RZ", update);
}

document.getElementById('zRange1').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      document.getElementById('zRange1Slider').value = this.value;
      AdjustRangeRZ();
    }
  }, false
);

document.getElementById('zRange1Slider').oninput = function(){
  document.getElementById('zRange1').value = this.value;
  AdjustRangeRZ();
}

document.getElementById('zRange2').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      document.getElementById('zRange2Slider').value = this.value;
      AdjustRangeRZ();
    }
  }, false
);

document.getElementById('zRange2Slider').oninput = function(){
  document.getElementById('zRange2').value = this.value;
  AdjustRangeRZ();
}

document.getElementById('rRange').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      let rrrr = parseInt(this.value);
      if ( rrrr < 1 ){
        document.getElementById('rRange').value = 1;
        rrrr = 1;
      }
      document.getElementById('rRangeSlider').value = rrrr;
      AdjustRangeRZ();
      
    }
  }, false
);

document.getElementById('rRangeSlider').oninput = function(){
  document.getElementById('rRange').value = this.value;
  AdjustRangeRZ();
}

document.getElementById('eRange').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      let rrrr = parseInt(this.value);
      if ( rrrr < 1 ){
        document.getElementById('eRange').value = 1;
        rrrr = 1;
      }
      document.getElementById('eRangeSlider').value = rrrr;
      AdjustRangeEZ();
    }
  }, false
);

document.getElementById('eRangeSlider').oninput = function(){
  document.getElementById('eRange').value = this.value;
  AdjustRangeEZ();
}

window.logMeThis = function(){
  SetSSType();
  CalculateEZ();
  CalculateRZ();
}

let FuncEx = window.logMeThis.bind(null, "Ex");
window.addEventListener('keypress', FuncEx);

let FuncThetaCM = window.logMeThis.bind(null, "thetaCM");
window.addEventListener('keypress', FuncThetaCM);

window.checkSSType = function(){
  SetSSType();
  CalculateEZ();
  CalThetaCM();
  CalculateRZ();
}

let FuncCheckSSType = window.checkSSType.bind(null, "SSType");
document.getElementById('HELIOS').addEventListener('click', FuncCheckSSType);
document.getElementById('SOLARIS').addEventListener('click', FuncCheckSSType);
document.getElementById('ISS').addEventListener('click', FuncCheckSSType);

document.getElementById('Ex0').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      CalThetaCM();
    }
  }, false
);

document.getElementById('thetaCMGate').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      CalThetaCM();
    }
  }, false
);

document.getElementById('XGate').addEventListener('keypress', 
  function(e){
    if(e.keyCode == 13){
      CalThetaCM();
    }
  }, false
);


function HalfCylinder(r, y, d, up){
 let a = [];
 let b = [];
 let c = [];

 let nStep = 60;
 let step = 2*r/nStep;

 for( let i = 0; i <= nStep ; i++){
  let x = -r + i * step;
  a.push(x);
  c.push(Math.sqrt(r*r - x*x) * up);
  b.push(y);
 }


 for( let i = 0; i <= nStep; i ++){
  let x = r - i * step;
  a.push(x);
  c.push(Math.sqrt(r*r - x*x) * up);
  b.push(y+d);
 }


 let haha = [];
 haha.push(a);
 haha.push(b);
 haha.push(c);

 return haha;
}

function detMesh(startPos, phi){
  let aaa = 110; // prepdist
  let len = 500; // detLen
  let www = 100; //det width

  let a =[];
  let b =[];
  let c =[];

  let cs = Math.cos(phi * Math.PI / 180);
  let ss = Math.sin(phi * Math.PI / 180);
  
  let A0 = aaa * cs - www / 2 * ss;
  let A1 = aaa * ss + www / 2 * cs;
  let B0 = aaa * cs + www / 2 * ss;
  let B1 = aaa * ss - www / 2 * cs;

  a.push(A0); b.push(A1); c.push(startPos);
  a.push(B0); b.push(B1); c.push(startPos);
  a.push(B0); b.push(B1); c.push(startPos + len);
  a.push(A0); b.push(A1); c.push(startPos + len);

  let haha = [];
  haha.push(a);
  haha.push(b);
  haha.push(c);

  return haha;

}

function Helix(theta, phi, rho, sign, nCyc){
 // sign = B-field
 // nCyc < 0 = updatream 

 let a = [];
 let b = [];
 let c = [];

 let deg = Math.PI/180;
 let nStep = 100;
 let ts = Math.tan(theta * deg);
 let zRange = nCyc * rho/ts * Math.PI * 2;
 let zStep = zRange/nStep;
 
 
 for( let i = 0; i < nStep; i++){
   let zzz;
   zzz = i * zStep;
   b.push(zzz);
   a.push( rho * ( Math.sin( ts * zzz / rho - phi * deg ) + Math.sin(phi * deg) ) );
   c.push( rho * sign * (Math.cos( ts * zzz / rho - phi * deg ) - Math.cos(phi * deg)) );
 }

 let haha = [];
 haha.push(a);
 haha.push(b);
 haha.push(c);
 
 return haha;

}


function Plot3D(){
 
 let u1 = HalfCylinder(bore, -1000, 2500., 1);
 let d1 = HalfCylinder(bore, -1000, 2500., -1);

 let line1 = Helix(40, 0,  80, -1, -1);
 let line2 = Helix(2, 180, 10, -1,  0.5);
 
 let data = [
   {type : 'mesh3d', x : u1[0], y : u1[1], z : u1[2], hoverinfo: 'none', opacity : 0.2, color : "#DDDDDD"}
   ,{type : 'mesh3d', x : d1[0], y : d1[1], z : d1[2], hoverinfo: 'none', opacity : 0.2, color : "#DDDDDD"}
   ,{type : 'scatter3d', mode : 'lines', x : line1[0], y : line1[1], z : line1[2], name : "haha"}
   ,{type : 'scatter3d', mode : 'lines', x : line2[0], y : line2[1], z : line2[2], name : "kaka"}
 ];
 

  /*
  let data = [];

  for( let i = 0; i < 1; i++){
    let haha = detMesh(-200, 360/6*i + 90);

    console.log(haha[0], "| ", haha[1], "| ", haha[2]);
    data.push(
       {type : 'mesh3d', x : haha[0], y : haha[1], z : haha[2], hoverinfo: 'none', opacity : 1.0, color : "#FF0000"} 
     );
  }*/

  let layout = {
    aspectmode: 'manual',
    aspectratio:{ x : 0.5, y : 0.5, z : 2.5 },
    margin: { l: 40, r: 40, b : 60, t : 40},
  };
  
  Plotly.newPlot('Plot_3D', data, layout);

}

GetMass();
CalConstants();
SetSSType();
CalculateEZ();
CalculateRZ();
AdjustRecoilPos();
CalThetaCM();

//Plot3D();
