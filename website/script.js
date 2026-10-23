const header = document.querySelector('[data-header]');
const menuButton = document.querySelector('[data-menu-button]');
const nav = document.querySelector('[data-nav]');

const dieFaces = {
  1: [5],
  2: [1, 9],
  3: [1, 5, 9],
  4: [1, 3, 7, 9],
  5: [1, 3, 5, 7, 9],
  6: [1, 3, 4, 6, 7, 9]
};
const rapidClickWindow = 5000;
const shatterClickCount = 8;
const dieClickHistory = new WeakMap();

const randomFace = (currentFace = 0) => {
  let nextFace = Math.floor(Math.random() * 6) + 1;
  while (nextFace === currentFace) nextFace = Math.floor(Math.random() * 6) + 1;
  return nextFace;
};

const renderDie = (die, face = randomFace()) => {
  die.dataset.face = String(face);
  die.querySelectorAll(':scope > i').forEach((pip, index) => {
    pip.classList.toggle('active', dieFaces[face].includes(index + 1));
  });
};

const rollDie = (die) => {
  renderDie(die, randomFace(Number(die.dataset.face)));
  die.classList.remove('is-rolling');
  void die.offsetWidth;
  die.classList.add('is-rolling');
  window.setTimeout(() => die.classList.remove('is-rolling'), 280);
};

const createFragments = (die) => {
  for (let position = 0; position < 9; position += 1) {
    const fragment = document.createElement('span');
    const row = Math.floor(position / 3);
    const column = position % 3;
    const xDirection = column - 1 || (Math.random() > 0.5 ? 0.35 : -0.35);
    const yDirection = row - 1 || (Math.random() > 0.5 ? 0.35 : -0.35);

    fragment.className = 'die-fragment';
    fragment.style.left = `${column * 33.333}%`;
    fragment.style.top = `${row * 33.333}%`;
    fragment.style.setProperty('--burst-x', `${Math.round(xDirection * (35 + Math.random() * 35))}px`);
    fragment.style.setProperty('--burst-y', `${Math.round(yDirection * (35 + Math.random() * 35))}px`);
    fragment.style.setProperty('--burst-r', `${Math.round((Math.random() - 0.5) * 240)}deg`);
    fragment.style.setProperty('--burst-delay', `${Math.round(Math.random() * 100)}ms`);
    fragment.style.borderRadius = [0, 2, 6, 8].includes(position) ? '4px' : '0';
    die.appendChild(fragment);
  }
};

const shatterDie = (die) => {
  if (die.dataset.busy === 'true') return;
  die.dataset.busy = 'true';
  dieClickHistory.set(die, []);
  createFragments(die);
  die.classList.remove('is-rolling');
  die.classList.add('is-shattering');

  window.setTimeout(() => die.classList.add('is-hidden'), 850);
  window.setTimeout(() => {
    die.querySelectorAll('.die-fragment').forEach((fragment) => fragment.remove());
    renderDie(die, randomFace(Number(die.dataset.face)));
    die.classList.remove('is-shattering', 'is-hidden');
    die.classList.add('is-restoring');
    die.dataset.busy = 'false';
    window.setTimeout(() => die.classList.remove('is-restoring'), 450);
  }, 2850);
};

const handleDieActivation = (event, die) => {
  event.preventDefault();
  event.stopPropagation();
  if (die.dataset.busy === 'true') return;

  const now = Date.now();
  const recentClicks = (dieClickHistory.get(die) || []).filter((time) => now - time <= rapidClickWindow);
  recentClicks.push(now);
  dieClickHistory.set(die, recentClicks);

  rollDie(die);
  if (recentClicks.length >= shatterClickCount) shatterDie(die);
};

document.querySelectorAll('[data-die]').forEach((die) => {
  for (let position = 1; position <= 9; position += 1) {
    const pip = document.createElement('i');
    die.appendChild(pip);
  }
  renderDie(die);

  const trigger = die.closest('[data-die-trigger]') || die;
  trigger.addEventListener('click', (event) => handleDieActivation(event, die));

  if (trigger === die) {
    die.addEventListener('keydown', (event) => {
      if (event.key !== 'Enter' && event.key !== ' ') return;
      handleDieActivation(event, die);
    });
  }
});

const closeMenu = () => {
  menuButton.setAttribute('aria-expanded', 'false');
  menuButton.setAttribute('aria-label', 'Open menu');
  nav.classList.remove('open');
  header.classList.remove('menu-open');
  document.body.classList.remove('menu-open');
};

menuButton.addEventListener('click', () => {
  const isOpen = menuButton.getAttribute('aria-expanded') === 'true';
  if (isOpen) {
    closeMenu();
    return;
  }

  menuButton.setAttribute('aria-expanded', 'true');
  menuButton.setAttribute('aria-label', 'Close menu');
  nav.classList.add('open');
  header.classList.add('menu-open');
  document.body.classList.add('menu-open');
});

nav.querySelectorAll('a').forEach((link) => link.addEventListener('click', closeMenu));
document.addEventListener('keydown', (event) => {
  if (event.key === 'Escape') closeMenu();
});

const updateHeader = () => header.classList.toggle('scrolled', window.scrollY > 16);
updateHeader();
window.addEventListener('scroll', updateHeader, { passive: true });

const reducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
const revealElements = document.querySelectorAll('.reveal');

if (reducedMotion || !('IntersectionObserver' in window)) {
  revealElements.forEach((element) => element.classList.add('visible'));
} else {
  const observer = new IntersectionObserver((entries) => {
    entries.forEach((entry) => {
      if (!entry.isIntersecting) return;
      entry.target.classList.add('visible');
      observer.unobserve(entry.target);
    });
  }, { threshold: 0.12 });

  revealElements.forEach((element) => observer.observe(element));
}

const copyStatus = document.querySelector('[data-copy-status]');
document.querySelectorAll('[data-copy]').forEach((button) => {
  button.addEventListener('click', () => {
    const address = button.dataset.copy;
    const currency = button.querySelector('span').textContent;
    const fallbackField = document.createElement('textarea');
    fallbackField.value = address;
    fallbackField.setAttribute('readonly', '');
    fallbackField.style.position = 'fixed';
    fallbackField.style.opacity = '0';
    document.body.appendChild(fallbackField);
    fallbackField.select();

    let copied = false;
    try {
      copied = document.execCommand('copy');
    } catch {
      copied = false;
    }
    fallbackField.remove();

    if (copied) {
      copyStatus.textContent = `${currency} address copied`;
      copyStatus.classList.add('success');
      return;
    }

    if (!navigator.clipboard) {
      copyStatus.textContent = 'Copy is unavailable — select the address manually';
      copyStatus.classList.remove('success');
      return;
    }

    navigator.clipboard.writeText(address).then(() => {
      copyStatus.textContent = `${currency} address copied`;
      copyStatus.classList.add('success');
    }).catch(() => {
      copyStatus.textContent = 'Copy is unavailable — select the address manually';
      copyStatus.classList.remove('success');
    });
  });
});

document.querySelector('[data-year]').textContent = new Date().getFullYear();
