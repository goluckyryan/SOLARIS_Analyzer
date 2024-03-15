let energy = [];
let jpi = [];
let Name;
let A;
let Sym;

function breakdownName(str) {
  const match = str.match(/^(\d+)([a-zA-Z]+)$/);
  if (match) {
    const numberPart = parseInt(match[1]);
    const stringPart = match[2];
    return { numberPart, stringPart };
  } else {
    return null; // If the input string doesn't match the expected format
  }
}

let Sn = 999;
let Sp = 999;
let Sa = 999;

function GetData(){

  Name = document.getElementById('ASym').value;
  let maxEx = parseFloat(document.getElementById('maxEx').value);

  let str = 'displayIsoData.py?ASym=' + Name + "&maxEx=" + maxEx;

  let client = new XMLHttpRequest();
  client.onreadystatechange = function() {
    let haha = client.responseText.split('\n');

    jpi = [];
    energy = [];
    haha.forEach(line =>{

      // console.log(line)
      if( line.includes("Sn:") ) {
        let pos1 = line.indexOf("Sn:");
        let pos2 = line.indexOf("MeV");
        Sn = parseFloat(line.substring(pos1+3, pos2));
      }
      if( line.includes("Sp:") ) {
        let pos1 = line.indexOf("Sp:");
        let pos2 = line.indexOf("MeV");
        Sp = parseFloat(line.substring(pos1+3, pos2));
      }
      if( line.includes("Sa:") ) {
        let pos1 = line.indexOf("Sa:");
        let pos2 = line.indexOf("MeV");
        Sa = parseFloat(line.substring(pos1+3, pos2));
      }

      if( line.includes("<tr><td style=")  && line.length != 0) {
        jpi.push(line.substring(95).slice(0,-10).trim());
        energy.push(parseFloat(line.substring(43,54).trim()));
        // console.log(jpi[jpi.length - 1] + ", " + energy[energy.length-1]);
      }
    });
  }
  client.open('GET', str, false);
  client.send();
  
}

function PlotLevels(){

  GetData();

  Plotly.purge("Plot_Levels");

  if( energy.length == 0 ) return;

  // console.log( Name + " | num. states : " + energy.length);
  
  const plotWidth = 300;
  const plotHeight = 600;
  const yMin = -1;

  let maxEx = parseFloat(document.getElementById('maxEx').value);
  const maxExExp = Math.max(...energy);

  // console.log(maxExExp);

  // let maxY = parseFloat(document.getElementById('plotRange').value);

  const fig = {
    data: [],
    layout: {
      plot_bgcolor: 'white',
      width: plotWidth,
      height: plotHeight,
      margin: { l: 0, r: 0, t: 0, b: 0 },
      showlegend: false,
      xaxis: {
        showline: false,
        visible: false,
        range: [-1, 2.5]
      },
      yaxis: {
        range: [yMin, maxEx + 2],
        showline: false,
        visible: false
      },
      annotations: []
    }
  };

  const l = energy.length;

  const fontSize = 14;
  const fontSizeMeV = fontSize / plotHeight * (maxExExp + 1 - yMin);

  let ypos = [];
  for( let i = 0; i < energy.length; i++) ypos.push(energy[i]);

  let noOverlap = false;
  let loop = 0;

  while (!noOverlap && loop < 2 * l) {
    for (let i = 1; i <= l; i++) {
      const diff = ypos[i] - ypos[i - 1];
      if (diff < fontSizeMeV) {
        ypos[i - 1] += (diff - fontSizeMeV) / 2;
        ypos[i] += (fontSizeMeV - diff) / 2;
        if (ypos[i - 1] < yMin + fontSizeMeV / 2) {
          ypos[i - 1] = yMin + fontSizeMeV / 2;
          ypos[i] = ypos[i - 1] + fontSizeMeV;
        }
      }
    }
    let count = 0;
    for (let i = 1; i <= l; i++) {
      const diff = ypos[i] - ypos[i - 1];
      if (diff > fontSizeMeV) {
        count++;
      }
    }
    if (count === l) {
      noOverlap = true;
    }
    loop++;
  }

  for (let i = 0; i < l; i++) {
    fig.data.push({
      x: [0, 1],
      y: [energy[i], energy[i]],
      mode: 'lines',
      line: { color: 'black', width: 1 }
    });

    fig.data.push({
      x: [1.03, 1.1, 1.19],
      y: [energy[i], ypos[i], ypos[i]],
      mode: 'lines',
      line: { color: 'gray', width: 1 }
    });

    // console.log(energy[i]+ ", " + ypos[i]);

    fig.layout.annotations.push({
      x: 1.2,
      y: ypos[i],
      text: `${energy[i].toFixed(3)}, ${jpi[i]}`,
      xanchor: 'left',
      font: { size: fontSize },
      showarrow: false
    });
  }

  console.log("Sn: " + Sn);
  console.log("Sp: " + Sp);
  console.log("Sa: " + Sa);

  let leftPos = -0.8;

  fig.data.push({
    x: [leftPos, 1],
    y: [Sn, Sn],
    mode: 'lines',
    line: { color: 'blue', width: 1 }
  });
  fig.layout.annotations.push({
    x: leftPos,
    y: Sn + fontSizeMeV/2,
    text: `${'Sn:'+Sn.toFixed(3)}`,
    xanchor: 'left',
    font: { size: fontSize, color: 'blue' },
    showarrow: false
  });

  fig.data.push({
    x: [leftPos, 1],
    y: [Sp, Sp],
    mode: 'lines',
    line: { color: 'red', width: 1 }
  });
  fig.layout.annotations.push({
    x: leftPos,
    y: Sp + fontSizeMeV/2,
    text: `${'Sp:'+Sp.toFixed(3)}`,
    xanchor: 'left',
    font: { size: fontSize, color: 'red' },
    showarrow: false
  });

  fig.data.push({
    x: [leftPos, 1],
    y: [Sa, Sa],
    mode: 'lines',
    line: { color: 'purple', width: 1 }
  });
  fig.layout.annotations.push({
    x: leftPos,
    y: Sa + fontSizeMeV/2,
    text: `${'Sa:'+Sa.toFixed(3)}`,
    xanchor: 'left',
    font: { size: fontSize, color: 'purple' },
    showarrow: false
  });

  // let NameYPos = (parseFloat(maxEx) + 2*fontSizeMeV);
  // console.log(NameYPos);

  let name2 = breakdownName(Name);
  
  fig.layout.annotations.push({
    x: 0.5,
    y: (maxEx + 1),
    text: "<sup>" + name2.numberPart +"</sup>" + name2.stringPart,
    font: { size: 2 * fontSize },
    showarrow: false
  });

  // Create the plot
  Plotly.newPlot('Plot_Levels', fig.data, fig.layout);

}